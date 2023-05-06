#ifndef _CODE_WRITER_H_INCLUDE_
#define _CODE_WRITER_H_INCLUDE_

#include "parser.h"
#include <stdio.h>

#define CODE_WRITER_VM_FILENAME_MAX_LENGTH (32)

typedef struct code_writer * CodeWriter;

CodeWriter CodeWriter_init(FILE *fpAsm);
void CodeWriter_setFileName(CodeWriter thisObject, char *fileName);
void CodeWriter_writeInit(CodeWriter thisObject);
void CodeWriter_writeArithmetic(CodeWriter thisObject, char *command);
void CodeWriter_writePushPop(
    CodeWriter thisObject,
    Parser_CommandType command,
    char *segment,
    int index
);
void CodeWriter_writeLabel(CodeWriter thisObject, char *label);
void CodeWriter_writeGoto(CodeWriter thisObject, char *label);
void CodeWriter_writeIf(CodeWriter thisObject, char *label);
void CodeWriter_writeCall(CodeWriter thisObject, char *functionName, int numArgs);
void CodeWriter_writeReturn(CodeWriter thisObject);
void CodeWriter_writeFunction(CodeWriter thisObject, char *functionName, int numLocals);
void CodeWriter_close(CodeWriter thisObject);

void fputslist(FILE* fp, ...);

void writeArithmethicAdd(FILE* fpAsm);
void writeArithmethicSub(FILE* fpAsm);
void writeArithmethicNeg(FILE* fpAsm);
void writeArithmethicEq(FILE* fpAsm, char *skipLabel);
void writeArithmethicGt(FILE* fpAsm, char *skipLabel);
void writeArithmethicLt(FILE* fpAsm, char *skipLabel);
void writeArithmethicAnd(FILE* fpAsm);
void writeArithmethicOr(FILE* fpAsm);
void writeArithmethicNot(FILE* fpAsm);

void writePushConstant(FILE* fpAsm, int index);
void writePushLocal(FILE* fpAsm, int index);
void writePopLocal(FILE* fpAsm, int index);
void writePushArgument(FILE* fpAsm, int index);
void writePopArgument(FILE* fpAsm, int index);
void writePushThis(FILE* fpAsm, int index);
void writePopThis(FILE* fpAsm, int index);
void writePushThat(FILE* fpAsm, int index);
void writePopThat(FILE* fpAsm, int index);
void writePushPointer(FILE* fpAsm, int index);
void writePopPointer(FILE* fpAsm, int index);
void writePushTemp(FILE* fpAsm, int index);
void writePopTemp(FILE* fpAsm, int index);
void writePushStatic(FILE* fpAsm, char *vmFileName, int index);
void writePopStatic(FILE* fpAsm, char *vmFileName, int index);

#endif