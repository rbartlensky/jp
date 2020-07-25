#define _GNU_SOURCE

#include "json.h"
#include "backtrace.h"

#define SKIP_WHITESPACE(ctx) if (consume_whitespace(ctx) == EARLY_EOF) { \
                fprintf(stderr, "Incomplete JSON?"); \
                return INVALID; \
        }

static char HEX_CHARS[22] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a',
        'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F'
};

static char DIGITS[10] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
};

enum INTERNAL_ERROR { OK = 0, EARLY_EOF = 1, INVALID = -1 };

typedef struct Context {
        int line;
        int col;
        FILE *fd;
        Backtrace bt;
} Context;

static int consume_whitespace(Context *ctx) {
        char c;
        while ((c = fgetc(ctx->fd))) {
                switch (c) {
                case '\n':
                        ctx->line++;
                        ctx->col = 0;
                        break;
                case '\t':
                case '\r':
                case ' ':
                        ctx->col++;
                        break;
                case EOF:
                        return EARLY_EOF;
                default:
                        // put the non-whitespace char back
                        ungetc(c, ctx->fd);
                        return OK;
                };
        }
        return OK;
}

static int consume_char(Context *ctx, char to_consume) {
        char c;
        while ((c = fgetc(ctx->fd))) {
                if (c == to_consume) {
                        ctx->col++;
                        return OK;
                }
                switch (c) {
                case EOF:
                        return EARLY_EOF;
                default:
                        ungetc(c, ctx->fd);
                        return INVALID;
                };
        }
        return OK;
}

static int test_status(int status, char c, Context *ctx) {
        switch (status) {
        case EARLY_EOF:
        case INVALID:
                return INVALID;
        default:
                return 0;
        }
}

static int consume_any(Context *ctx, char *chars, int start, int len) {
        int i;
        for (i = start; i < len; ++i) {
                if (consume_char(ctx, HEX_CHARS[i]) == OK) {
                        return OK;
                }
        }
        return INVALID;
}

static int match_number(Context *ctx) {
        if (consume_char(ctx, '-') == OK) {
                if (consume_any(ctx, DIGITS, 1, sizeof(DIGITS)) != OK) {
                        return INVALID;
                }
                while (consume_any(ctx, DIGITS, 0, sizeof(DIGITS)) == OK);
        } else {
                if (consume_char(ctx, '0') != OK) {
                        return INVALID;
                }
        }
        if (consume_char(ctx, '.') == OK) {
                while (consume_any(ctx, DIGITS, 0, sizeof(DIGITS)) == OK);
        }
        if (consume_char(ctx, 'E') == OK || consume_char(ctx, 'e') == OK) {
                if (consume_char(ctx, '-') != OK) {
                        // don't really care what the output is
                        consume_char(ctx, '+');
                }
                while (consume_any(ctx, DIGITS, 0, sizeof(DIGITS)) == OK);
        }
        return OK;
}

static int match_string(Context *ctx) {
        char c, c2;
        int status, i;
        // match left '"'
        if ((status = test_status(consume_char(ctx, '"'), '"', ctx)) != OK) {
                return status;
        }
        while ((c = fgetc(ctx->fd))) {
                switch (c) {
                case EOF:
                        return EARLY_EOF;
                case '\b':
                case '\f':
                case '\r':
                case '\t':
                case '\n':
                        // controls characters are not valid in a string
                        return INVALID;
                case '"':
                        // we finished parsing the string
                        ctx->col++;
                        return OK;
                case '\\':
                        ctx->col++;
                        c2 = fgetc(ctx->fd);
                        switch(c2) {
                        case '"':
                        case '\\':
                        case '/':
                        case 'b':
                        case 'f':
                        case 'n':
                        case 'r':
                        case 't':
                                // we escaped a control char
                                break;
                        case 'u':
                                // match exactly 4 hex digits
                                for (i = 0; i < 4; ++i) {
                                        if (consume_any(ctx, HEX_CHARS, 0, sizeof(HEX_CHARS)) != OK) {
                                                return INVALID;
                                        }
                                }
                                break;
                        default:
                                return INVALID;
                        }
                default:
                        ctx->col++;
                        break;
                };
        }
}

static int match_sequence(Context *ctx, char *val, int len, int *matched) {
        int i;
        for (i = 0; i < len; ++i) {
                if (consume_char(ctx, val[i]) != OK) {
                        *matched = i;
                        char *msg;
                        asprintf(&msg, "Failed to match \"%s\" at line: %d, col: %d",
                                 val, ctx->line, ctx->col);
                        bt_push(&ctx->bt, msg);
                        return INVALID;
                }
        }
        *matched = len;
        return OK;
}

static int match_value(Context *ctx);

static int match_json_body(Context *ctx) {
        if (match_string(ctx) != OK) {
                return INVALID;
        }
        SKIP_WHITESPACE(ctx);
        if (test_status(consume_char(ctx, ':'), ':', ctx) == INVALID) {
                return INVALID;
        }
        SKIP_WHITESPACE(ctx);
        if (match_value(ctx) != OK) {
                char *err;
                asprintf(&err, "Failed to match value at line: %d, col: %d",
                         ctx->line,
                         ctx->col);
                bt_push(&ctx->bt, err);
                return INVALID;
        }
        SKIP_WHITESPACE(ctx);
        if (consume_char(ctx, '}') == OK) {
                return OK;
        }
        if (consume_char(ctx, ',') != OK) {
                return INVALID;
        }
        SKIP_WHITESPACE(ctx);
        return match_json_body(ctx);
}

static int match_object(Context *ctx) {
        if (test_status(consume_char(ctx, '{'), '{', ctx) == INVALID) {
                return INVALID;
        }
        SKIP_WHITESPACE(ctx);
        // now we either match a '}' or a "string" : value
        if (consume_char(ctx, '}') == OK) {
                return OK;
        }
        return match_json_body(ctx);
}

static int match_array_body(Context *ctx) {
        if (match_value(ctx) != OK) {
                return INVALID;
        }
        SKIP_WHITESPACE(ctx);
        if (consume_char(ctx, ']') == OK) {
                return OK;
        }
        if (consume_char(ctx, ',') != OK) {
                return INVALID;
        }
        SKIP_WHITESPACE(ctx);
        return match_array_body(ctx);
}

static int match_array(Context *ctx) {
        if (test_status(consume_char(ctx, '['), '[', ctx) == INVALID) {
                return INVALID;
        }
        SKIP_WHITESPACE(ctx);
        if (consume_char(ctx, ']') == OK) {
                return OK;
        }
        return match_array_body(ctx);
}

static int match_value(Context *ctx) {
        static int (*matchers[4])(Context*) = { match_string, match_object, match_number, match_array};
        int old_len = ctx->bt.len;
        int i;
        for (i = 0; i < 4; ++i) {
                if (matchers[i](ctx) == OK) {
                        while (old_len < ctx->bt.len) {
                                bt_pop(&ctx->bt);
                        }
                        return OK;
                }
        }
        int matched = 0;
        static char *strs[3] = { "true", "false", "null" };
        static int lens[3] = { 4, 5, 4 };
        for (i = 0; i < 3; ++i) {
                switch(match_sequence(ctx, strs[i], lens[i], &matched)) {
                case OK:
                        while (old_len < ctx->bt.len) {
                                bt_pop(&ctx->bt);
                        }
                        return OK;
                case EARLY_EOF:
                case INVALID:
                        if (matched > 0) {
                                ctx->col -= matched;
                                return INVALID;
                        }
                default:
                        break;
                }
        }
        return INVALID;
}

int jp_check(FILE *fd) {
        Backtrace bt = bt_new();
        Context ctx = { 1, 0, fd, bt };
        // XXX: is an empty file a valid json?
        if (consume_whitespace(&ctx) == EARLY_EOF) {
                bt_free(&ctx.bt);
                return 0;
        }
        if (match_object(&ctx) != OK) {
                bt_print(&ctx.bt);
                bt_free(&ctx.bt);
                return JP_BAD_JSON;
        }
        bt_free(&ctx.bt);
        return 0;
}
