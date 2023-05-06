#include "code_writer_private.h"
#include "code_writer.h"
#include <stdarg.h>

#define PUSH_POP_INDEX_MAX_DIGIT   (6)
#define PUSH_POP_SYMBOL_MAX_LENGTH (CODE_WRITER_VM_FILENAME_MAX_LENGTH + PUSH_POP_INDEX_MAX_DIGIT + 1)

void writeArithmethicUnaryOperation(FILE* fpAsm, char *comp);
void writeArithmethicBinaryOperation(FILE* fpAsm, char *comp);
void writeArithmethicCondition(FILE* fpAsm, char *skipLabel, char *jump);

void writePushSymbol(FILE* fpAsm, char *symbol, int index);
void writePopSymbol(FILE* fpAsm, char *symbol, int index);
void writePushRegister(FILE* fpAsm, int registerNumber);
void writePopRegister(FILE* fpAsm, int registerNumber);

void writeArithmethicAdd(FILE* fpAsm) { writeArithmethicBinaryOperation(fpAsm, "D+M"); }
void writeArithmethicSub(FILE* fpAsm) { writeArithmethicBinaryOperation(fpAsm, "M-D"); }
void writeArithmethicNeg(FILE* fpAsm) { writeArithmethicUnaryOperation(fpAsm, "-M"); }
void writeArithmethicEq(FILE* fpAsm, char *skipLabel) { writeArithmethicCondition(fpAsm, skipLabel, "JEQ"); }
void writeArithmethicGt(FILE* fpAsm, char *skipLabel) { writeArithmethicCondition(fpAsm, skipLabel, "JGT"); }
void writeArithmethicLt(FILE* fpAsm, char *skipLabel) { writeArithmethicCondition(fpAsm, skipLabel, "JLT"); }
void writeArithmethicAnd(FILE* fpAsm) { writeArithmethicBinaryOperation(fpAsm, "D&M"); }
void writeArithmethicOr(FILE* fpAsm)  { writeArithmethicBinaryOperation(fpAsm, "D|M"); }
void writeArithmethicNot(FILE* fpAsm) { writeArithmethicUnaryOperation(fpAsm, "!M"); }

void writePushConstant(FILE* fpAsm, int index)
{
    char indexStr[PUSH_POP_INDEX_MAX_DIGIT + 1];
    sprintf(indexStr, "%d", index);

    fputsMultipleLines(
        fpAsm,
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

void writePushLocal(FILE* fpAsm, int index) { writePushSymbol(fpAsm, "LCL", index); }
void writePopLocal(FILE* fpAsm, int index)  { writePopSymbol(fpAsm, "LCL", index); }

void writePushArgument(FILE* fpAsm, int index) { writePushSymbol(fpAsm, "ARG", index); }
void writePopArgument(FILE* fpAsm, int index)  { writePopSymbol(fpAsm, "ARG", index); }

void writePushThis(FILE* fpAsm, int index) { writePushSymbol(fpAsm, "THIS", index); }
void writePopThis(FILE* fpAsm, int index)  { writePopSymbol(fpAsm, "THIS", index); }

void writePushThat(FILE* fpAsm, int index) { writePushSymbol(fpAsm, "THAT", index); }
void writePopThat(FILE* fpAsm, int index)  { writePopSymbol(fpAsm, "THAT", index); }

void writePushPointer(FILE* fpAsm, int index) { writePushRegister(fpAsm, 3 + index); }
void writePopPointer(FILE* fpAsm, int index)  { writePopRegister(fpAsm, 3 + index); }

void writePushTemp(FILE* fpAsm, int index) { writePushRegister(fpAsm, 5 + index); }
void writePopTemp(FILE* fpAsm, int index)  { writePopRegister(fpAsm, 5 + index); }

void writePushStatic(FILE* fpAsm, char *vmFileName, int index)
{
    char symbol[PUSH_POP_SYMBOL_MAX_LENGTH + 1];
    sprintf(symbol, "%s.%d", vmFileName, index);

    fputsMultipleLines(
        fpAsm,
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

void writePopStatic(FILE* fpAsm, char *vmFileName, int index)
{
    char symbol[PUSH_POP_SYMBOL_MAX_LENGTH + 1];
    sprintf(symbol, "%s.%d", vmFileName, index);

    fputsMultipleLines(
        fpAsm,
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

void writeArithmethicUnaryOperation(FILE* fpAsm, char *comp)
{
    fputsMultipleLines(
        fpAsm,
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

void writeArithmethicBinaryOperation(FILE* fpAsm, char *comp)
{
    fputsMultipleLines(
        fpAsm,
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

void writeArithmethicCondition(FILE* fpAsm, char *skipLabel, char *jump)
{
    fputsMultipleLines(
        fpAsm,
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

void writePushSymbol(FILE* fpAsm, char *symbol, int index)
{
    char indexStr[PUSH_POP_INDEX_MAX_DIGIT + 1];
    sprintf(indexStr, "%d", index);

    fputsMultipleLines(
        fpAsm,
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

void writePopSymbol(FILE* fpAsm, char *symbol, int index)
{
    char indexStr[PUSH_POP_INDEX_MAX_DIGIT + 1];
    sprintf(indexStr, "%d", index);

    fputsMultipleLines(
        fpAsm,
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

void writePushRegister(FILE* fpAsm, int registerNumber)
{
    char symbol[8];
    sprintf(symbol, "R%d", registerNumber);

    fputsMultipleLines(
        fpAsm,
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

void writePopRegister(FILE* fpAsm, int registerNumber)
{
    char symbol[8];
    sprintf(symbol, "R%d", registerNumber);

    fputsMultipleLines(
        fpAsm,
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

void fputsMultipleLines(FILE* fp, ...)
{
    char* string;

    va_list args;
    va_start(args, fp);

    while ((string = va_arg(args, char*)) != NULL) {
        fputs(string, fp);
    }

    va_end(args);
}