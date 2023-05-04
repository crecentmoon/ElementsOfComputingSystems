#ifndef SYMBOL_H
#define SYMBOL_H

#include "assembler.h"
#include "parser.h"
#define MAX_SYMBOLS 65536

typedef struct {
    char symbol[MAX_LINE_LEN];
    int address;
} symbol_t;

void add_entry(symbol_t *table, int size, char *symbol, int address);

int get_address(symbol_t *table, int size, char *symbol);

#endif
