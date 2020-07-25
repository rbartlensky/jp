typedef struct Backtrace {
        char **bt;
        int len;
        int capacity;
} Backtrace;

/* Create a new backtrace object. */
Backtrace bt_new();

/* Push a new string to the backtrace. The backtrace owns the string. */
void bt_push(Backtrace* bt, char *str);

/* Pop a string from the backtrace. */
void bt_pop(Backtrace *bt);

/* Free the backtrace and the strings that were pushed to it. */
void bt_free(Backtrace *bt);

/* Print the backtrace to stderr. */
void bt_print(Backtrace *bt);
