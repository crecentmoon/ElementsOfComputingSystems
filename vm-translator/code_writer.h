#ifndef CODE_WRITER
#define CODE_WRITER

#include "Parser.h"
#include <stdio.h>

#define CODE_WRITER_VM_FILENAME_MAX_LENGTH (32)

struct CodeWriter
{
    FILE* asmFilePath;
    char vmFileName[CODE_WRITER_VM_FILENAME_MAX_LENGTH + 1];
    int arithmeticEqCount;
    int arithmeticGtCount;
    int arithmeticLtCount;
};

struct CodeWriter CodeWriter_construct(FILE *asmFilePath);

void CodeWriter_setFileName(struct CodeWriter thisObject, char *fileName);

void CodeWriter_writeArithmetic(struct CodeWriter thisObject, char *command);

void CodeWriter_writePushPop(struct CodeWriter thisObject, Parser_CommandType command, char *segment, int index);

void CodeWriter_close(struct CodeWriter thisObject);

#endif