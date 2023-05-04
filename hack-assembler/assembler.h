#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#define MAX_LINE_LEN 256
#define MAX_SYMBOLS 10000

typedef enum {
    A_COMMAND,
    C_COMMAND,
    L_COMMAND,
    NO_COMMAND
} command_type_t;

int assemble(FILE *input, FILE *output);

#endif /* ASSEMBLER_H */
