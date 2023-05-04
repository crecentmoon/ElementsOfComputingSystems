#ifndef CODE_H
#define CODE_H

#include "symbol_table.h"

int generate_a_command(char *line, symbol_t *symbol_table, int num_symbols);

char generate_c_command(char *line);

char* get_dest_code(char *dest);

char* get_comp_code(char *comp);

char* get_jump_code(char *jump);

#endif
