#ifndef PARSER_H
#define PARSER_H

#include "assembler.h"

command_type_t get_command_type(char *line);

char *get_label(char *line);

char *get_symbol(char *line);

char *get_dest(char *line);

char *get_comp(char *line);

char *get_jump(char *line);

#endif
