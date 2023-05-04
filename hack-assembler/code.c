#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "code.h"

int generate_a_command(char *line, symbol_t *symbol_table, int num_symbols) {
    char symbol[MAX_LINE_LEN];
    sscanf(line, "@%s", symbol);
    if (isdigit(symbol[0])) {
        return atoi(symbol);
    }
    else {
        return get_address(symbol_table, num_symbols, symbol);
    }
}

char generate_c_command(char *line) {
    char dest[MAX_LINE_LEN];
    char comp[MAX_LINE_LEN];
    char jump[MAX_LINE_LEN];
    dest[0] = '\0';
    comp[0] = '\0';
    jump[0] = '\0';
    char *equal = strchr(line, '=');
    char *semicolon = strchr(line, ';');
    if (equal != NULL) {
        strncpy(dest, line, equal - line);
        dest[equal - line] = '\0';
        strncpy(comp, equal + 1, strlen(line) - (equal - line) - 1);
        comp[strlen(line) - (equal - line) - 1] = '\0';
    }
    else {
        strncpy(comp, line, semicolon - line);
        comp[semicolon - line] = '\0';
    }
    if (semicolon != NULL) {
        strncpy(jump, semicolon + 1, strlen(semicolon + 1) - 1);
        jump[strlen(semicolon + 1) - 1] = '\0';
    }
    return (char)((0b111 << 3) | (get_dest_code(dest)) | get_comp_code(comp) | get_jump_code(jump));
}

char* get_dest_code(char *dest)
{
    if (strcmp(dest, "null") == 0) {
        return "000";
    } else if (strcmp(dest, "M") == 0) {
        return "001";
    } else if (strcmp(dest, "D") == 0) {
        return "010";
    } else if (strcmp(dest, "MD") == 0) {
        return "011";
    } else if (strcmp(dest, "A") == 0) {
        return "100";
    } else if (strcmp(dest, "AM") == 0) {
        return "101";
    } else if (strcmp(dest, "AD") == 0) {
        return "110";
    } else if (strcmp(dest, "AMD") == 0) {
        return "111";
    }

    return "";
}

char* get_comp_code(char *comp)
{
    if (strcmp(comp, "0") == 0) {
        return "0101010";
    } else if (strcmp(comp, "1") == 0) {
        return "0111111";
    } else if (strcmp(comp, "-1") == 0) {
        return "0111010";
    } else if (strcmp(comp, "D") == 0) {
        return "0001100";
    } else if (strcmp(comp, "A") == 0 || strcmp(comp, "M") == 0) {
        return "0110000";
    } else if (strcmp(comp, "!D") == 0) {
        return "0001101";
    } else if (strcmp(comp, "!A") == 0 || strcmp(comp, "!M") == 0) {
        return "0110001";
    } else if (strcmp(comp, "-D") == 0) {
        return "0001111";
    } else if (strcmp(comp, "-A") == 0 || strcmp(comp, "-M") == 0) {
        return "0110011";
    } else if (strcmp(comp, "D+1") == 0) {
        return "0011111";
    } else if (strcmp(comp, "A+1") == 0 || strcmp(comp, "M+1") == 0) {
        return "0110111";
    } else if (strcmp(comp, "D-1") == 0) {
        return "0001110";
    } else if (strcmp(comp, "A-1") == 0 || strcmp(comp, "M-1") == 0) {
        return "0110010";
    } else if (strcmp(comp, "D+A") == 0 || strcmp(comp, "D+M") == 0) {
        return "0000010";
    } else if (strcmp(comp, "D-A") == 0 || strcmp(comp, "D-M") == 0) {
        return "0010011";
    } else if (strcmp(comp, "A-D") == 0 || strcmp(comp, "M-D") == 0) {
        return "0000111";
    } else if (strcmp(comp, "D&A") == 0 || strcmp(comp, "D&M") == 0) {
        return "0000000";
    } else if (strcmp(comp, "D|A") == 0 || strcmp(comp, "D|M") == 0) {
        return "0010101";
    }

    return "";
}
    
char* get_jump_code(char *jump)
{
    if (strcmp(jump, "null") == 0) {
        return "000";
    } else if (strcmp(jump, "JGT") == 0) {
        return "001";
    } else if (strcmp(jump, "JEQ") == 0) {
        return "010";
    } else if (strcmp(jump, "JGE") == 0) {
        return "011";
    } else if (strcmp(jump, "JLT") == 0) {
        return "100";
    } else if (strcmp(jump, "JNE") == 0) {
        return "101";
    } else if (strcmp(jump, "JLE") == 0) {
        return "110";
    } else if (strcmp(jump, "JMP") == 0) {
        return "111";
    }

    return "";
}