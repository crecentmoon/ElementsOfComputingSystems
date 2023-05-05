#include "code_writer.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define PUSH_CONSTANT_INDEX_MAX_DIGIT (6)
#define ARITHMETIC_SKIP_LABEL_MAX_LENGTH (CODE_WRITER_VM_FILENAME_MAX_LENGTH + 24)
#define PUSH_POP_INDEX_MAX_DIGIT   (6)
#define PUSH_POP_SYMBOL_MAX_LENGTH (CODE_WRITER_VM_FILENAME_MAX_LENGTH + PUSH_POP_INDEX_MAX_DIGIT + 1)

static void writeArithmethicBinaryOperation(FILE* asmFilePath, char *comp);
static void writeArithmethicUnaryOperation(FILE* asmFilePath, char *comp);
static void writeArithmethicCondition(FILE* asmFilePath, char *skipLabel, char *jump);

static void writePushSymbol(FILE* asmFilePath, char *symbol, int index);
static void writePopSymbol(FILE* asmFilePath, char *symbol, int index);
static void writePushRegister(FILE* asmFilePath, int registerNumber);
static void writePopRegister(FILE* asmFilePath, int registerNumber);

static void writePushLocal(FILE* asmFilePath, int index);
static void writePopLocal(FILE* asmFilePath, int index);
static void writePushArgument(FILE* asmFilePath, int index);
static void writePopArgument(FILE* asmFilePath, int index);
static void writePushThis(FILE* asmFilePath, int index);
static void writePopThis(FILE* asmFilePath, int index);
static void writePushThat(FILE* asmFilePath, int index);
static void writePopThat(FILE* asmFilePath, int index);

static void writePushPointer(FILE* asmFilePath, int index);
static void writePopPointer(FILE* asmFilePath, int index);
static void writePushTemp(FILE* asmFilePath, int index);
static void writePopTemp(FILE* asmFilePath, int index);
static void writePushStatic(FILE* asmFilePath, char *vmFileName, int index);
static void writePopStatic(FILE* asmFilePath, char *vmFileName, int index);

// Arithmetic command operations
static void writeArithmethicAdd(FILE* asmFilePath);
static void writeArithmethicSub(FILE* asmFilePath);
static void writeArithmethicNeg(FILE* asmFilePath);
static void writeArithmethicEq(FILE* asmFilePath, char *skipLabel);
static void writeArithmethicGt(FILE* asmFilePath, char *skipLabel);
static void writeArithmethicLt(FILE* asmFilePath, char *skipLabel);
static void writeArithmethicAnd(FILE* asmFilePath);
static void writeArithmethicOr(FILE* asmFilePath);
static void writeArithmethicNot(FILE* asmFilePath);

static void writePushConstant(FILE* asmFilePath, int index);

static void writeArithmethicEqNext(struct CodeWriter obj);
static void writeArithmethicGtNext(struct CodeWriter obj);
static void writeArithmethicLtNext(struct CodeWriter obj);

static void fputsMultipleLines(FILE* fp, ...);

struct CodeWriter CodeWriter_construct(FILE *asmFilePath)
{
    static struct CodeWriter obj;

    obj.asmFilePath = asmFilePath;
    CodeWriter_setFileName(obj, "");

    return obj;
}

void CodeWriter_setFileName(struct CodeWriter obj, char *fileName)
{
    strcpy(obj.vmFileName, fileName);
    obj.arithmeticEqCount = 0;
    obj.arithmeticGtCount = 0;
    obj.arithmeticLtCount = 0;
}

void CodeWriter_writeArithmetic(struct CodeWriter obj, char *command)
{
    if (strcmp(command, "add") == 0) writeArithmethicAdd(obj.asmFilePath);
    else if (strcmp(command, "sub") == 0) writeArithmethicSub(obj.asmFilePath);
    else if (strcmp(command, "neg") == 0) writeArithmethicNeg(obj.asmFilePath);
    else if (strcmp(command,  "eq") == 0) writeArithmethicEqNext(obj);
    else if (strcmp(command,  "gt") == 0) writeArithmethicGtNext(obj);
    else if (strcmp(command,  "lt") == 0) writeArithmethicLtNext(obj);
    else if (strcmp(command, "and") == 0) writeArithmethicAnd(obj.asmFilePath);
    else if (strcmp(command,  "or") == 0) writeArithmethicOr(obj.asmFilePath);
    else if (strcmp(command, "not") == 0) writeArithmethicNot(obj.asmFilePath);
}

void CodeWriter_writePushPop(
    struct CodeWriter cw,
    Parser_CommandType command,
    char *segment,
    int index
) {
    switch (command) {
    case PARSER_COMMAND_TYPE_C_PUSH:
        if (strcmp(segment, "constant") == 0) writePushConstant(cw.asmFilePath, index);
        else if (strcmp(segment,    "local") == 0) writePushLocal(cw.asmFilePath, index);
        else if (strcmp(segment, "argument") == 0) writePushArgument(cw.asmFilePath, index);
        else if (strcmp(segment,     "this") == 0) writePushThis(cw.asmFilePath, index);
        else if (strcmp(segment,     "that") == 0) writePushThat(cw.asmFilePath, index);
        else if (strcmp(segment,  "pointer") == 0) writePushPointer(cw.asmFilePath, index);
        else if (strcmp(segment,     "temp") == 0) writePushTemp(cw.asmFilePath, index);
        else if (strcmp(segment,   "static") == 0) writePushStatic(cw.asmFilePath, cw.vmFileName, index);
        break;
    case PARSER_COMMAND_TYPE_C_POP:
        if (strcmp(segment,    "local") == 0) writePopLocal(cw.asmFilePath, index);
        else if (strcmp(segment, "argument") == 0) writePopArgument(cw.asmFilePath, index);
        else if (strcmp(segment,     "this") == 0) writePopThis(cw.asmFilePath, index);
        else if (strcmp(segment,     "that") == 0) writePopThat(cw.asmFilePath, index);
        else if (strcmp(segment,  "pointer") == 0) writePopPointer(cw.asmFilePath, index);
        else if (strcmp(segment,     "temp") == 0) writePopTemp(cw.asmFilePath, index);
        else if (strcmp(segment,   "static") == 0) writePopStatic(cw.asmFilePath, cw.vmFileName, index);
        break;
    default:
        break;
    }
} 

void CodeWriter_close(struct CodeWriter cw)
{
    fclose(cw.asmFilePath);
}

void writeArithmethicEqNext(struct CodeWriter cw)
{
    char skipLabel[ARITHMETIC_SKIP_LABEL_MAX_LENGTH + 1];
    sprintf(skipLabel, "%s.SKIP_EQ.%d", cw.vmFileName, cw.arithmeticEqCount);
    writeArithmethicEq(cw.asmFilePath, skipLabel);
    cw.arithmeticEqCount++;
}

void writeArithmethicGtNext(struct CodeWriter cw)
{
    char skipLabel[ARITHMETIC_SKIP_LABEL_MAX_LENGTH + 1];
    sprintf(skipLabel, "%s.SKIP_GT.%d", cw.vmFileName, cw.arithmeticGtCount);
    writeArithmethicGt(cw.asmFilePath, skipLabel);
    cw.arithmeticGtCount++;
}

void writeArithmethicLtNext(struct CodeWriter cw)
{
    char skipLabel[ARITHMETIC_SKIP_LABEL_MAX_LENGTH + 1];
    sprintf(skipLabel, "%s.SKIP_LT.%d", cw.vmFileName, cw.arithmeticLtCount);
    writeArithmethicLt(cw.asmFilePath, skipLabel);
    cw.arithmeticLtCount++;
}

static void fputsMultipleLines(FILE* filePath, ...)
{
    char* string;

    va_list args;
    va_start(args, filePath);

    while ((string = va_arg(args, char*)) != NULL) {
        fputs(string, filePath);
    }

    va_end(args);
}

static void writeArithmethicAdd(FILE* asmFilePath) { writeArithmethicBinaryOperation(asmFilePath, "D+M"); }
static void writeArithmethicSub(FILE* asmFilePath) { writeArithmethicBinaryOperation(asmFilePath, "M-D"); }
static void writeArithmethicNeg(FILE* asmFilePath) { writeArithmethicUnaryOperation(asmFilePath, "-M"); }
static void writeArithmethicEq(FILE* asmFilePath, char *skipLabel) { writeArithmethicCondition(asmFilePath, skipLabel, "JEQ"); }
static void writeArithmethicGt(FILE* asmFilePath, char *skipLabel) { writeArithmethicCondition(asmFilePath, skipLabel, "JGT"); }
static void writeArithmethicLt(FILE* asmFilePath, char *skipLabel) { writeArithmethicCondition(asmFilePath, skipLabel, "JLT"); }
static void writeArithmethicAnd(FILE* asmFilePath) { writeArithmethicBinaryOperation(asmFilePath, "D&M"); }
static void writeArithmethicOr(FILE* asmFilePath)  { writeArithmethicBinaryOperation(asmFilePath, "D|M"); }
static void writeArithmethicNot(FILE* asmFilePath) { writeArithmethicUnaryOperation(asmFilePath, "!M"); }

static void writePushConstant(FILE* asmFilePath, int index)
{
    char indexStr[PUSH_CONSTANT_INDEX_MAX_DIGIT + 1];
    sprintf(indexStr, "%d", index);

    fputsMultipleLines(
        asmFilePath,
        "// push constant\n",
        "@", indexStr, "\n",
        "D=A\n",
        "@SP\n",
        "A=M\n",
        "M=D\n",
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}

// symbol: LCL, ARG, THIS, THAT,
static void writePushLocal(FILE* asmFilePath, int index) { writePushSymbol(asmFilePath, "LCL", index); }
static void writePopLocal(FILE* asmFilePath, int index)  { writePopSymbol(asmFilePath, "LCL", index); }
static void writePushArgument(FILE* asmFilePath, int index) { writePushSymbol(asmFilePath, "ARG", index); }
static void writePopArgument(FILE* asmFilePath, int index)  { writePopSymbol(asmFilePath, "ARG", index); }
static void writePushThis(FILE* asmFilePath, int index) { writePushSymbol(asmFilePath, "THIS", index); }
static void writePopThis(FILE* asmFilePath, int index)  { writePopSymbol(asmFilePath, "THIS", index); }
static void writePushThat(FILE* asmFilePath, int index) { writePushSymbol(asmFilePath, "THAT", index); }
static void writePopThat(FILE* asmFilePath, int index)  { writePopSymbol(asmFilePath, "THAT", index); }

static void writePushPointer(FILE* asmFilePath, int index) { writePushRegister(asmFilePath, 3 + index); }
static void writePopPointer(FILE* asmFilePath, int index)  { writePopRegister(asmFilePath, 3 + index); }
static void writePushTemp(FILE* asmFilePath, int index) { writePushRegister(asmFilePath, 5 + index); }
static void writePopTemp(FILE* asmFilePath, int index)  { writePopRegister(asmFilePath, 5 + index); }

static void writePushStatic(FILE* asmFilePath, char *vmFileName, int index)
{
    char symbol[PUSH_POP_SYMBOL_MAX_LENGTH + 1];
    sprintf(symbol, "%s.%d", vmFileName, index);

    fputsMultipleLines(
        asmFilePath,
        "@", symbol, "\n",
        "D=M\n",
        "@SP\n",
        "A=M\n",
        "M=D\n",
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}

static void writePopStatic(FILE* asmFilePath, char *vmFileName, int index)
{
    char symbol[PUSH_POP_SYMBOL_MAX_LENGTH + 1];
    sprintf(symbol, "%s.%d", vmFileName, index);

    fputsMultipleLines(
        asmFilePath,
        "@SP\n",
        "M=M-1\n",
        "@SP\n",
        "A=M\n",
        "D=M\n",
        "@", symbol, "\n",
        "M=D\n",
        NULL
    );
}
static void writeArithmethicUnaryOperation(FILE* asmFilePath, char *comp)
{
    fputsMultipleLines(
        asmFilePath,
        "// UnaryOperation ", comp, "\n",
        "@SP\n",
        "M=M-1\n",
        "A=M\n",
        "M=", comp, "\n",
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}
static void writeArithmethicBinaryOperation(FILE* asmFilePath, char *comp)
{
    fputsMultipleLines(
        asmFilePath,
        "// BinaryOperation ", comp, "\n",
        "@SP\n",
        "M=M-1\n",
        "A=M\n",
        "D=M\n",
        "@SP\n",
        "M=M-1\n",
        "A=M\n",
        "M=", comp, "\n",
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}
static void writeArithmethicCondition(FILE* asmFilePath, char *skipLabel, char *jump)
{
    fputsMultipleLines(
        asmFilePath,
        "// Condition ", jump, "\n",
        "@SP\n",
        "M=M-1\n",
        "A=M\n",
        "D=M\n",
        "@SP\n",
        "M=M-1\n",
        "A=M\n",
        "D=M-D\n",
        "@SP\n",
        "A=M\n",
        "M=-1\n",
        "@", skipLabel, "\n",
        "D;", jump, "\n",
        "@SP\n",
        "A=M\n",
        "M=0\n",
        "(", skipLabel, ")\n",
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}

void writePushSymbol(FILE* asmFilePath, char *symbol, int index)
{
    char indexStr[PUSH_POP_INDEX_MAX_DIGIT + 1];
    sprintf(indexStr, "%d", index);

    fputsMultipleLines(
        asmFilePath,
        "// push symbol ", symbol, " ", indexStr, "\n",
        "@", indexStr, "\n",
        "D=A\n",
        "@", symbol, "\n",
        "D=D+M\n",
        "@R13\n",
        "M=D\n",
        "A=M\n",
        "D=M\n",
        "@SP\n",
        "A=M\n",
        "M=D\n",
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}

void writePopSymbol(FILE* asmFilePath, char *symbol, int index)
{
    char indexStr[PUSH_POP_INDEX_MAX_DIGIT + 1];
    sprintf(indexStr, "%d", index);

    fputsMultipleLines(
        asmFilePath,
        "@SP\n",
        "M=M-1\n",
        "@", indexStr, "\n",
        "D=A\n",
        "@", symbol, "\n",
        "D=D+M\n",
        "@R13\n",
        "M=D\n",
        "@SP\n",
        "A=M\n",
        "D=M\n",
        "@R13\n",
        "A=M\n",
        "M=D\n",
        NULL
    );
}

void writePushRegister(FILE* asmFilePath, int registerNumber)
{
    char symbol[8];
    sprintf(symbol, "R%d", registerNumber);

    fputsMultipleLines(
        asmFilePath,
        "@", symbol, "\n",
        "D=M\n",
        "@SP\n",
        "A=M\n",
        "M=D\n",
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}

void writePopRegister(FILE* asmFilePath, int registerNumber)
{
    char symbol[8];
    sprintf(symbol, "R%d", registerNumber);

    fputsMultipleLines(
        asmFilePath,
        "@SP\n",
        "M=M-1\n",
        "@SP\n",
        "A=M\n",
        "D=M\n",
        "@", symbol, "\n",
        "M=D\n",
        NULL
    );
}