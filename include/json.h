#include <stdio.h>

enum JP_ERROR { JP_BAD_JSON = -1, JP_OK = 0 };

/**
    Check whether the file is a valid JSON. Returns JP_OK if it is valid, or
    JP_BAD_JSON otherwise.
*/
int jp_check(FILE *fd);
