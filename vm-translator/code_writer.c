#include "code_writer.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define PUSH_CONSTANT_INDEX_MAX_DIGIT (6)
#define ARITHMETIC_SKIP_LABEL_MAX_LENGTH (CODE_WRITER_VM_FILENAME_MAX_LENGTH + 24)

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
static void writeArithmethicBinaryOperation(FILE* asmFilePath, char *comp);
static void writeArithmethicUnaryOperation(FILE* asmFilePath, char *comp);
static void writeArithmethicCondition(FILE* asmFilePath, char *skipLabel, char *jump);
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

void CodeWriter_writePushPop(struct CodeWriter obj, Parser_CommandType command, char *segment, int index) 
{
    if (command == PARSER_COMMAND_TYPE_C_PUSH) {
        if (strcmp(segment, "constant") == 0) {
            writePushConstant(obj.asmFilePath, index);
        }
    }
}

void CodeWriter_close(struct CodeWriter obj)
{
    fclose(obj.asmFilePath);
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

// pop y, pop x, push (x + y)
static void writeArithmethicAdd(FILE* asmFilePath) { writeArithmethicBinaryOperation(asmFilePath, "D+M"); }
// pop y, pop x, push (x - y)
static void writeArithmethicSub(FILE* asmFilePath) { writeArithmethicBinaryOperation(asmFilePath, "M-D"); }
// pop y, push (-y)
static void writeArithmethicNeg(FILE* asmFilePath) { writeArithmethicUnaryOperation(asmFilePath, "-M"); }

// pop y, pop x, push (x == y ? -1 : 0)
static void writeArithmethicEq(FILE* asmFilePath, char *skipLabel) { writeArithmethicCondition(asmFilePath, skipLabel, "JEQ"); }
// pop y, pop x, push (x >  y ? -1 : 0)
static void writeArithmethicGt(FILE* asmFilePath, char *skipLabel) { writeArithmethicCondition(asmFilePath, skipLabel, "JGT"); }
// pop y, pop x, push (x <  y ? -1 : 0)
static void writeArithmethicLt(FILE* asmFilePath, char *skipLabel) { writeArithmethicCondition(asmFilePath, skipLabel, "JLT"); }

// pop y, pop x, push (x and y)
static void writeArithmethicAnd(FILE* asmFilePath) { writeArithmethicBinaryOperation(asmFilePath, "D&M"); }
// pop y, pop x, push (x or y)
static void writeArithmethicOr(FILE* asmFilePath)  { writeArithmethicBinaryOperation(asmFilePath, "D|M"); }
// pop y, push (not y)
static void writeArithmethicNot(FILE* asmFilePath) { writeArithmethicUnaryOperation(asmFilePath, "!M"); }

// Unary operation (M <- y)
static void writeArithmethicUnaryOperation(FILE* asmFilePath, char *comp)
{
    fputsMultipleLines(
        asmFilePath,
        "// UnaryOperation ", comp, "\n",
        // Memory[SP] -= 1
        "@SP\n",
        "M=M-1\n",
        // y <- Memory[Memory[SP]]
        "A=M\n",
        // Memory[Memory[SP]] <- comp
        "M=", comp, "\n",
        // Memory[SP] += 1
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}

// push index
static void writePushConstant(FILE* asmFilePath, int index)
{
    char indexStr[PUSH_CONSTANT_INDEX_MAX_DIGIT + 1];
    sprintf(indexStr, "%d", index);

    fputsMultipleLines(
        asmFilePath,
        "// push constant\n",
        // Memory[Memory[SP]] <- index
        "@", indexStr, "\n",
        "D=A\n",
        "@SP\n",
        "A=M\n",
        "M=D\n",
        // Memory[SP] += 1
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}

// Binary operation (M <- x, D <- y)
static void writeArithmethicBinaryOperation(FILE* asmFilePath, char *comp)
{
    fputsMultipleLines(
        asmFilePath,
        "// BinaryOperation ", comp, "\n",
        // Memory[SP] -= 1
        "@SP\n",
        "M=M-1\n",
        // y <- Memory[Memory[SP]]
        "A=M\n",
        "D=M\n",
        // Memory[SP] -= 1
        "@SP\n",
        "M=M-1\n",
        // x <- Memory[Memory[SP]]
        "A=M\n",
        // Memory[Memory[SP]] <- comp
        "M=", comp, "\n",
        // Memory[SP] += 1
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}

// Condition (comp <- x - y)
static void writeArithmethicCondition(FILE* asmFilePath, char *skipLabel, char *jump)
{
    fputsMultipleLines(
        asmFilePath,
        "// Condition ", jump, "\n",
        // Memory[SP] -= 1
        "@SP\n",
        "M=M-1\n",
        // y <- Memory[Memory[SP]]
        "A=M\n",
        "D=M\n",
        // Memory[SP] -= 1
        "@SP\n",
        "M=M-1\n",
        // x <- Memory[Memory[SP]]
        "A=M\n",
        // Register <- x - y
        "D=M-D\n",
        // Memory[Memory[SP]] <- -1(true/0xFFFF)
        "@SP\n",
        "A=M\n",
        "M=-1\n",
        // if jump(x - y) then goto skipLabel
        "@", skipLabel, "\n",
        "D;", jump, "\n",
        // Memory[Memory[SP] <- 0(false/0x0000)
        "@SP\n",
        "A=M\n",
        "M=0\n",
        // skipLabel:
        "(", skipLabel, ")\n",
        // Memory[SP] += 1
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}

