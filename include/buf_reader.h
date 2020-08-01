#include <stdio.h>

enum BUF {
        BUF_LEN = 256
};

typedef struct BufReader {
        FILE *fd;
        int cursor;
        int eof;
        char buf[BUF_LEN];
} BufReader;

BufReader buf_new(FILE *fd);

char buf_getc(BufReader *r);

char buf_peek(BufReader *r);
