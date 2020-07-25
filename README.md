# jp -- Json Parser

This is my attempt at creating a JSON validator in C.

## Future work

* Backtraces (why/where did it fail?):
** an initial version of this is now included
* Better parsing (stop making use of recursive functions, better parsing
techniques, etc.)

## Why another validator?

Learn about parsing, and practice my C skills.

## Why C?

I am writing a lot of Rust code, and I've always wanted to learn C. Might also
implement this in Rust and compare the two implementations performance wise.

## How to build and run

* Build: `make`
* Run: `./jp -f /path/to/file.json`

Input:

```json
{

    "dsa\u1233"

    :


    tru

}
```

Output:

```
0: Failed to match value at line: 8, col: 4
1: Failed to match "true" at line: 8, col: 7
```
