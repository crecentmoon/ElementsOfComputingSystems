#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

command_type_t get_command_type(char *line) {
    char first_char = line[0];

    if (first_char == '@') {
        return A_COMMAND;
    }
    else if (first_char == '(') {
        return L_COMMAND;
    }
    else {
        return C_COMMAND;
    }
}

char *get_label(char *line) {
    char *label = malloc(MAX_LINE_LEN);

    sscanf(line, "(%[^)])", label);

    return label;
}

char *get_symbol(char *line) {
    char *symbol = malloc(MAX_LINE_LEN);

    if (get_command_type(line) == A_COMMAND) {
        sscanf(line, "@%[^0123456789\n]", symbol);
    }
    else if (get_command_type(line) == L_COMMAND) {
        sscanf(line, "(%[^)])", symbol);
    }

    return symbol;
}

char *get_dest(char *line) {
    char *dest = malloc(MAX_LINE_LEN);
    char *equal = strchr(line, '=');

    if (equal != NULL) {
        strncpy(dest, line, equal - line);
        dest[equal - line] = '\0';
    }
    else {
        dest[0] = '\0';
    }

    return dest;
}

char *get_comp(char *line) {
    char *comp = malloc(MAX_LINE_LEN);
    char *equal = strchr(line, '=');
    char *semicolon = strchr(line, ';');

    if (equal != NULL) {
        strncpy(comp, equal + 1, semicolon - equal - 1);
    }
    else {
        strncpy(comp, line, semicolon - line);
    }

    comp[semicolon - equal - 1] = '\0';
    
    return comp;
}

char *get_jump(char *line) {
    char *jump = malloc(MAX_LINE_LEN);
    char *semicolon = strchr(line, ';');
    if (semicolon != NULL) {
        strncpy(jump, semicolon + 1, strlen(semicolon + 1) - 1);
        jump[strlen(semicolon + 1) - 1] = '\0';
    }
    else {
        jump[0] = '\0';
    }
    return jump;
}
