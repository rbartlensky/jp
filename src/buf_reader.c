#include "buf_reader.h"

BufReader buf_new(FILE *fd) {
        BufReader ret = { fd, 0, 0 };
        int i, eof;
        eof = fread(&ret.buf, 1, BUF_LEN, fd);
        ret.eof = eof;
        return ret;
}

char buf_getc(BufReader *br) {
        if (br->cursor > br->eof) {
                return EOF;
        }
        char ret = br->buf[br->cursor];
        if (br->cursor == BUF_LEN - 1) {
                int i, eof;
                eof = fread(&br->buf, 1, BUF_LEN, br->fd);
                br->cursor = 0;
                br->eof = eof;
        } else {
                br->cursor++;
        }
        return ret;
}

char buf_peek(BufReader *br) {
        if (br->cursor > br->eof) {
                return EOF;
        }
        return br->buf[br->cursor];
}
