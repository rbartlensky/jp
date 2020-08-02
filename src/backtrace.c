#include "backtrace.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Backtrace bt_new() {
        int initial_capacity = 16;
        char **bt = malloc(sizeof(char*) * 16);
        if (!bt) {
                fprintf(stderr, "Failed to allocate memory for backtrace.");
                exit(1);
        }
        Backtrace ret = { bt, 0, initial_capacity };
        return ret;
}

void bt_push(Backtrace* bt, char *str) {
        if (bt->capacity <= bt->len) {
                char **bt_bigger = realloc(bt->bt, sizeof(char*) * bt->capacity * 2);
                if (!bt_bigger) {
                        fprintf(stderr, "Failed to allocate memory for backtrace.");
                        exit(1);
                }
                bt->capacity *= 2;
                bt->bt = bt_bigger;
        }
        bt->bt[bt->len++] = str;
}

void bt_pop(Backtrace *bt) {
        if (bt->len > 0) {
                free(bt->bt[bt->len - 1]);
                bt->len--;
        }
}

void bt_free(Backtrace *bt) {
        int i;
        for (i = 0; i < bt->len; ++i) {
                free(bt->bt[i]);
        }
        free(bt->bt);
        Backtrace new_bt = { NULL, 0, 0 };
        *bt = new_bt;
}

void bt_print(Backtrace *bt) {
        int i;
        for (i = 0; i < bt->len; ++i) {
                fprintf(stderr, "%d: %s\n", i, bt->bt[bt->len - i - 1]);
        }
}
