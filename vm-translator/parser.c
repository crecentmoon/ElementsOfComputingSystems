#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define IF_CMP_RET(var, str, ret) if (strcmp(var, str) == 0) return ret

static bool isSpace(FILE *vmFilePath);
static bool isComment(FILE *vmFilePath);
static bool isEndOfFile(FILE *vmFilePath);
static bool isEndOfLine(FILE *vmFilePath);
static bool isToken(FILE *vmFilePath);
static void skipSpaces(FILE *vmFilePath);
static void skipEndOFLines(FILE *vmFilePath);
static void skipComment(FILE *vmFilePath);
static void moveNextAdvance(FILE *vmFilePath);
static void getToken(FILE *vmFilePath, char *token);

struct Parser Parser_construct(FILE *vmFilePath)
{
    static struct Parser thisObject;

    thisObject.vmFilePath = vmFilePath;
    fseek(thisObject.vmFilePath, 0L, SEEK_SET);
    moveNextAdvance(thisObject.vmFilePath);
    strcpy(thisObject.command, "");
    strcpy(thisObject.arg1,    "");
    strcpy(thisObject.arg2,    "");

    return thisObject;
}

bool Parser_hasMoreCommands(struct Parser thisObject)
{
    return ! isEndOfFile(thisObject.vmFilePath);
}

void Parser_advance(struct Parser thisObject)
{
    getToken(thisObject.vmFilePath, thisObject.command);

    switch (Parser_commandType(thisObject)) {
    case PARSER_COMMAND_TYPE_C_PUSH:
        skipSpaces(thisObject.vmFilePath);
        getToken(thisObject.vmFilePath, thisObject.arg1);
        skipSpaces(thisObject.vmFilePath);
        getToken(thisObject.vmFilePath, thisObject.arg2);
        break;
    default:
        strcpy(thisObject.arg1, "");
        strcpy(thisObject.arg2, "");
        break;
    }

    moveNextAdvance(thisObject.vmFilePath);
}

Parser_CommandType Parser_commandType(struct Parser thisObject)
{
    IF_CMP_RET(thisObject.command,  "add", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject.command,  "sub", PARSER_COMMAND_TYPE_C_ARITHMETIC);   
    IF_CMP_RET(thisObject.command,  "neg", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject.command,   "eq", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject.command,   "gt", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject.command,   "lt", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject.command,  "and", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject.command,   "or", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject.command,  "not", PARSER_COMMAND_TYPE_C_ARITHMETIC);
    IF_CMP_RET(thisObject.command, "push", PARSER_COMMAND_TYPE_C_PUSH);

    return -1;
}

void Parser_arg1(struct Parser thisObject, char *arg1)
{
    if (Parser_commandType(thisObject) == PARSER_COMMAND_TYPE_C_ARITHMETIC) {
        strcpy(arg1, thisObject.command);
    } else if (Parser_commandType(thisObject) == PARSER_COMMAND_TYPE_C_PUSH) {
        strcpy(arg1, thisObject.arg1);
    }
}

int Parser_arg2(struct Parser thisObject)
{
    if (Parser_commandType(thisObject) == PARSER_COMMAND_TYPE_C_PUSH) {
        return atoi(thisObject.arg2);
    }
    return -1;
}

static bool isSpace(FILE *vmFilePath)
{
    int c = fgetc(vmFilePath);
    ungetc(c, vmFilePath);

    if ((char)c == ' ' || (char)c == '\t') {
        return true;
    }
    return false;
}

static void skipSpaces(FILE *vmFilePath)
{
    while (isSpace(vmFilePath)) {
        fgetc(vmFilePath);
    }
}

static bool isComment(FILE *vmFilePath)
{
    int c1 = fgetc(vmFilePath);
    if ((char)c1 != '/') {
        ungetc(c1, vmFilePath);
        return false;
    }
    int c2 = fgetc(vmFilePath);
    ungetc(c2, vmFilePath);
    ungetc(c1, vmFilePath);
    if ((char)c2 == '/') {
        return true;
    }
    return false;
}

static bool isEndOfFile(FILE *vmFilePath)
{
    int c = fgetc(vmFilePath);
    ungetc(c, vmFilePath);

    if (c == EOF) {
        return true;
    }
    return false;
}

static bool isEndOfLine(FILE *vmFilePath)
{
    int c = fgetc(vmFilePath);
    ungetc(c, vmFilePath);
   
    if ((char)c == '\n' || (char)c == '\r') {
        return true;
    }
    return false;
}

static bool isToken(FILE *vmFilePath)
{
    return ! (isSpace(vmFilePath) || isEndOfFile(vmFilePath) || isEndOfLine(vmFilePath) || isComment(vmFilePath));
}

static void skipEndOFLines(FILE *vmFilePath)
{
    while (isEndOfLine(vmFilePath)) {
        fgetc(vmFilePath);
    }
}

static void skipComment(FILE *vmFilePath)
{
    if (isComment(vmFilePath)) {
        do {
            fgetc(vmFilePath);
        }
        while (! (isEndOfLine(vmFilePath) || isEndOfFile(vmFilePath)));
    }
}

static void moveNextAdvance(FILE *vmFilePath)
{
    do {
        skipEndOFLines(vmFilePath);
        skipSpaces(vmFilePath);
        skipComment(vmFilePath);
    }
    while (isEndOfLine(vmFilePath));
}

static void getToken(FILE *vmFilePath, char *token)
{
    int i = 0;
    while (isToken(vmFilePath)) {
        int c = fgetc(vmFilePath);
        token[i] = (char)c;
        i++;
    }
    token[i] = '\0';
}

