#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "code.h"
#include "symbol_table.h"
#include "assembler.h"

int assemble(FILE *input, FILE *output) {
    symbol_t symbol_table[MAX_SYMBOLS];
    int num_symbols = 0;
    int next_address = 0;
    char line[MAX_LINE_LEN];

    while (fgets(line, MAX_LINE_LEN, input) != NULL) {
        char *comment = strchr(line, '/');
        if (comment != NULL) {
            *comment = '\0';
        }

        if (strlen(line) == 0 || isspace(line[0])) {
            continue;
        }

        command_type_t type = get_command_type(line);
        if (type == L_COMMAND) {
            add_entry(symbol_table, num_symbols, get_label(line), next_address);
        }
        else {
            next_address++;
        }
    }

    
    add_entry(symbol_table, num_symbols, "SP", 0);
    add_entry(symbol_table, num_symbols, "LCL", 1);
    add_entry(symbol_table, num_symbols, "ARG", 2);
    add_entry(symbol_table, num_symbols, "THIS", 3);
    add_entry(symbol_table, num_symbols, "THAT", 4);
    for (int i = 0; i < 16; i++) {
        char symbol[10];
        sprintf(symbol, "R%d", i);
        add_entry(symbol_table, num_symbols, symbol, i);
    }
    add_entry(symbol_table, num_symbols, "SCREEN", 0x4000);
    add_entry(symbol_table, num_symbols, "KBD", 0x6000);

    fseek(input, 0, SEEK_SET);
    while (fgets(line, MAX_LINE_LEN, input) != NULL) {
        char *comment = strchr(line, '/');
        if (comment != NULL) {
            *comment = '\0';
        }
        
        if (strlen(line) == 0 || isspace(line[0])) {
            continue;
        }

        command_type_t type = get_command_type(line);
        if (type == A_COMMAND) {
            int address = generate_a_command(line, symbol_table, num_symbols);
            fprintf(output, "%016x\n", address);
        }
        else if (type == C_COMMAND) {
            int binary = generate_c_command(line);
            fprintf(output, "%016x\n", binary);
        }
    }
    
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: assembler <input_file> <output_file>\n");
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[2], "w");

    if (input == NULL) {
        printf("Error: could not open input file\n");
        return 1;
    } 
    else if (output == NULL) {
        printf("Error: could not open output file\n");
        return 1;
    }

    int result = assemble(input, output);

    fclose(input);
    fclose(output);

    return result;
}
