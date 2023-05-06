#include "parser.h"
#include <string.h>
#include <stdlib.h>

#define IF_CMP_RET(var, str, ret) if (strcmp(var, str) == 0) return ret

static bool isSpace(FILE *fpVm);
static bool isComment(FILE *fpVm);
static bool isEndOfFile(FILE *fpVm);
static bool isEndOfLine(FILE *fpVm);
static bool isToken(FILE *fpVm);
static void skipSpaces(FILE *fpVm);
static void skipEndOFLines(FILE *fpVm);
static void skipComment(FILE *fpVm);
static void moveNextAdvance(FILE *fpVm);
static void getToken(FILE *fpVm, char *token);

struct parser
{
    FILE* fpVm;
    char command[PARSER_COMMAND_MAX_LENGTH + 1];
    char arg1[PARSER_ARG1_MAX_LENGTH + 1];
    char arg2[PARSER_ARG2_MAX_LENGTH + 1];
};

Parser Parser_init(FILE *fpVm)
{
    static struct parser thisObject;

    thisObject.fpVm = fpVm;
    fseek(thisObject.fpVm, 0L, SEEK_SET);
    moveNextAdvance(thisObject.fpVm);
    strcpy(thisObject.command, "");
    strcpy(thisObject.arg1,    "");
    strcpy(thisObject.arg2,    "");

    return &thisObject;
}

bool Parser_hasMoreCommands(Parser thisObject)
{
    return ! isEndOfFile(thisObject->fpVm);
}

void Parser_advance(Parser thisObject)
{
    getToken(thisObject->fpVm, thisObject->command);

    switch (Parser_commandType(thisObject)) {
    case PARSER_COMMAND_TYPE_C_PUSH:
    case PARSER_COMMAND_TYPE_C_POP:
    case PARSER_COMMAND_TYPE_C_FUNCTION:
    case PARSER_COMMAND_TYPE_C_CALL:
        skipSpaces(thisObject->fpVm);
        getToken(thisObject->fpVm, thisObject->arg1);
        skipSpaces(thisObject->fpVm);
        getToken(thisObject->fpVm, thisObject->arg2);
        break;
    case PARSER_COMMAND_TYPE_C_LABEL:
    case PARSER_COMMAND_TYPE_C_GOTO:
    case PARSER_COMMAND_TYPE_C_IF:
        skipSpaces(thisObject->fpVm);
        getToken(thisObject->fpVm, thisObject->arg1);
        strcpy(thisObject->arg2, "");
        break;
    case PARSER_COMMAND_TYPE_C_RETURN:
    default:
        strcpy(thisObject->arg1, "");
        strcpy(thisObject->arg2, "");
        break;
    }

    moveNextAdvance(thisObject->fpVm);
}

Parser_CommandType Parser_commandType(Parser thisObject)
{
    IF_CMP_RET(thisObject->command,      "add", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject->command,      "sub", PARSER_COMMAND_TYPE_C_ARITHMETIC);   
    IF_CMP_RET(thisObject->command,      "neg", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject->command,       "eq", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject->command,       "gt", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject->command,       "lt", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject->command,      "and", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject->command,       "or", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject->command,      "not", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject->command,     "push", PARSER_COMMAND_TYPE_C_PUSH);
    IF_CMP_RET(thisObject->command,      "pop", PARSER_COMMAND_TYPE_C_POP);
    IF_CMP_RET(thisObject->command,    "label", PARSER_COMMAND_TYPE_C_LABEL);
    IF_CMP_RET(thisObject->command,     "goto", PARSER_COMMAND_TYPE_C_GOTO);
    IF_CMP_RET(thisObject->command,  "if-goto", PARSER_COMMAND_TYPE_C_IF);
    IF_CMP_RET(thisObject->command, "function", PARSER_COMMAND_TYPE_C_FUNCTION);
    IF_CMP_RET(thisObject->command,   "return", PARSER_COMMAND_TYPE_C_RETURN);
    IF_CMP_RET(thisObject->command,     "call", PARSER_COMMAND_TYPE_C_CALL);

    return -1;
}

void Parser_arg1(Parser thisObject, char *arg1)
{
    if (Parser_commandType(thisObject) == PARSER_COMMAND_TYPE_C_ARITHMETIC) {
        strcpy(arg1, thisObject->command);
    } else {
        strcpy(arg1, thisObject->arg1);
    }
}

int Parser_arg2(Parser thisObject)
{
    if (Parser_commandType(thisObject) == PARSER_COMMAND_TYPE_C_PUSH ||
        Parser_commandType(thisObject) == PARSER_COMMAND_TYPE_C_POP ||
        Parser_commandType(thisObject) == PARSER_COMMAND_TYPE_C_FUNCTION ||
        Parser_commandType(thisObject) == PARSER_COMMAND_TYPE_C_CALL) {
        return atoi(thisObject->arg2);
    }
    return -1;
}

static bool isSpace(FILE *fpVm)
{
    int c = fgetc(fpVm);
    ungetc(c, fpVm);

    if ((char)c == ' ' || (char)c == '\t') {
        return true;
    }
    return false;
}

static void skipSpaces(FILE *fpVm)
{
    while (isSpace(fpVm)) {
        fgetc(fpVm);
    }
}

static bool isComment(FILE *fpVm)
{
    int c1 = fgetc(fpVm);
    if ((char)c1 != '/') {
        ungetc(c1, fpVm);
        return false;
    }
    int c2 = fgetc(fpVm);
    ungetc(c2, fpVm);
    ungetc(c1, fpVm);
    if ((char)c2 == '/') {
        return true;
    }
    return false;
}

static bool isEndOfFile(FILE *fpVm)
{
    int c = fgetc(fpVm);
    ungetc(c, fpVm);

    if (c == EOF) {
        return true;
    }
    return false;
}

static bool isEndOfLine(FILE *fpVm)
{
    int c = fgetc(fpVm);
    ungetc(c, fpVm);
   
    if ((char)c == '\n' || (char)c == '\r') {
        return true;
    }
    return false;
}

static bool isToken(FILE *fpVm)
{
    return ! (isSpace(fpVm) || isEndOfFile(fpVm) || isEndOfLine(fpVm) || isComment(fpVm));
}

static void skipEndOFLines(FILE *fpVm)
{
    while (isEndOfLine(fpVm)) {
        fgetc(fpVm);
    }
}

static void skipComment(FILE *fpVm)
{
    if (isComment(fpVm)) {
        do {
            fgetc(fpVm);
        }
        while (! (isEndOfLine(fpVm) || isEndOfFile(fpVm)));
    }
}

static void moveNextAdvance(FILE *fpVm)
{
    do {
        skipEndOFLines(fpVm);
        skipSpaces(fpVm);
        skipComment(fpVm);
    }
    while (isEndOfLine(fpVm));
}

static void getToken(FILE *fpVm, char *token)
{
    int i = 0;
    while (isToken(fpVm)) {
        int c = fgetc(fpVm);
        token[i] = (char)c;
        i++;
    }
    token[i] = '\0';
}