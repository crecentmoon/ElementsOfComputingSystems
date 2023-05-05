#ifndef PARSER
#define PARSER

#include <stdio.h>
#include <stdbool.h>

#define PARSER_COMMAND_MAX_LENGTH (4)
#define PARSER_ARG1_MAX_LENGTH    (8)
#define PARSER_ARG2_MAX_LENGTH    (16)

struct Parser
{
    FILE* vmFilePath;
    char command[PARSER_COMMAND_MAX_LENGTH + 1];
    char arg1[PARSER_ARG1_MAX_LENGTH + 1];
    char arg2[PARSER_ARG2_MAX_LENGTH + 1];
};

typedef enum {
    PARSER_COMMAND_TYPE_C_ARITHMETIC = 1,
    PARSER_COMMAND_TYPE_C_PUSH,
    PARSER_COMMAND_TYPE_C_POP,
} Parser_CommandType;

struct Parser Parser_construct(FILE *vmFilePath);
bool Parser_hasMoreCommands(struct Parser thisObject);
void Parser_advance(struct Parser thisObject);
Parser_CommandType Parser_commandType(struct Parser thisObject);
void Parser_arg1(struct Parser thisObject, char *arg1);
int Parser_arg2(struct Parser thisObject);

#endif