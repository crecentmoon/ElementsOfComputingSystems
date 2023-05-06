#include "code_writer.h"
#include "code_writer_private.h"
#include <string.h>
#include <stdarg.h>

#define PUSH_POP_INDEX_MAX_DIGIT   (6)
#define PUSH_POP_SYMBOL_MAX_LENGTH (CODE_WRITER_VM_FILENAME_MAX_LENGTH + PUSH_POP_INDEX_MAX_DIGIT + 1)
#define ARITHMETIC_SKIP_LABEL_MAX_LENGTH (CODE_WRITER_VM_FILENAME_MAX_LENGTH + 24)
#define LABEL_SYMBOL_MAX_LENGTH (CODE_WRITER_VM_FILENAME_MAX_LENGTH + 24)

void writeArithmethicEqNext(CodeWriter thisObject);
void writeArithmethicGtNext(CodeWriter thisObject);
void writeArithmethicLtNext(CodeWriter thisObject);
void writePushRegisterByName(FILE* fpAsm, char *registerName);

static void writeArithmethicUnaryOperation(FILE* fpAsm, char *comp);
static void writeArithmethicBinaryOperation(FILE* fpAsm, char *comp);
static void writeArithmethicCondition(FILE* fpAsm, char *skipLabel, char *jump);

static void writePushSymbol(FILE* fpAsm, char *symbol, int index);
static void writePopSymbol(FILE* fpAsm, char *symbol, int index);
static void writePushRegister(FILE* fpAsm, int registerNumber);
static void writePopRegister(FILE* fpAsm, int registerNumber);

struct code_writer
{
    FILE* fpAsm;
    char vmFileName[CODE_WRITER_VM_FILENAME_MAX_LENGTH + 1];
    int arithmeticEqCount;
    int arithmeticGtCount;
    int arithmeticLtCount;
    int callCount;
};

CodeWriter CodeWriter_init(FILE *fpAsm)
{
    static struct code_writer thisObject;

    thisObject.fpAsm = fpAsm;
    CodeWriter_setFileName(&thisObject, "");
    thisObject.callCount = 0;

    return &thisObject;
}

void CodeWriter_setFileName(CodeWriter thisObject, char *fileName)
{
    strcpy(thisObject->vmFileName, fileName);
    thisObject->arithmeticEqCount = 0;
    thisObject->arithmeticGtCount = 0;
    thisObject->arithmeticLtCount = 0;
}

void CodeWriter_writeInit(CodeWriter thisObject)
{
    fputslist(
        thisObject->fpAsm,
        // Memory[SP] <- 256
        "@256\n",
        "D=A\n",
        "@SP\n",
        "M=D\n",
        NULL
    );
    // call Sys.init 0
    CodeWriter_writeCall(thisObject, "Sys.init", 0);
}

void CodeWriter_writeArithmetic(CodeWriter thisObject, char *command)
{
         if (strcmp(command, "add") == 0) writeArithmethicAdd(thisObject->fpAsm);
    else if (strcmp(command, "sub") == 0) writeArithmethicSub(thisObject->fpAsm);
    else if (strcmp(command, "neg") == 0) writeArithmethicNeg(thisObject->fpAsm);
    else if (strcmp(command,  "eq") == 0) writeArithmethicEqNext(thisObject);
    else if (strcmp(command,  "gt") == 0) writeArithmethicGtNext(thisObject);
    else if (strcmp(command,  "lt") == 0) writeArithmethicLtNext(thisObject);
    else if (strcmp(command, "and") == 0) writeArithmethicAnd(thisObject->fpAsm);
    else if (strcmp(command,  "or") == 0) writeArithmethicOr(thisObject->fpAsm);
    else if (strcmp(command, "not") == 0) writeArithmethicNot(thisObject->fpAsm);
}

void CodeWriter_writePushPop(
    CodeWriter thisObject,
    Parser_CommandType command,
    char *segment,
    int index
) {
    switch (command) {
    case PARSER_COMMAND_TYPE_C_PUSH:
             if (strcmp(segment, "constant") == 0) writePushConstant(thisObject->fpAsm, index);
        else if (strcmp(segment,    "local") == 0) writePushLocal(thisObject->fpAsm, index);
        else if (strcmp(segment, "argument") == 0) writePushArgument(thisObject->fpAsm, index);
        else if (strcmp(segment,     "this") == 0) writePushThis(thisObject->fpAsm, index);
        else if (strcmp(segment,     "that") == 0) writePushThat(thisObject->fpAsm, index);
        else if (strcmp(segment,  "pointer") == 0) writePushPointer(thisObject->fpAsm, index);
        else if (strcmp(segment,     "temp") == 0) writePushTemp(thisObject->fpAsm, index);
        else if (strcmp(segment,   "static") == 0) writePushStatic(thisObject->fpAsm, thisObject->vmFileName, index);
        break;
    case PARSER_COMMAND_TYPE_C_POP:
             if (strcmp(segment,    "local") == 0) writePopLocal(thisObject->fpAsm, index);
        else if (strcmp(segment, "argument") == 0) writePopArgument(thisObject->fpAsm, index);
        else if (strcmp(segment,     "this") == 0) writePopThis(thisObject->fpAsm, index);
        else if (strcmp(segment,     "that") == 0) writePopThat(thisObject->fpAsm, index);
        else if (strcmp(segment,  "pointer") == 0) writePopPointer(thisObject->fpAsm, index);
        else if (strcmp(segment,     "temp") == 0) writePopTemp(thisObject->fpAsm, index);
        else if (strcmp(segment,   "static") == 0) writePopStatic(thisObject->fpAsm, thisObject->vmFileName, index);
        break;
    default:
        break;
    }
}

void CodeWriter_writeLabel(CodeWriter thisObject, char *label)
{
    char labelSymbol[LABEL_SYMBOL_MAX_LENGTH + 1];
    sprintf(labelSymbol, "%s$%s", thisObject->vmFileName, label);

    fputslist(
        thisObject->fpAsm,
        "(", labelSymbol, ")\n",
        NULL
    );
}

void CodeWriter_writeGoto(CodeWriter thisObject, char *label)
{
    char labelSymbol[LABEL_SYMBOL_MAX_LENGTH + 1];
    sprintf(labelSymbol, "%s$%s", thisObject->vmFileName, label);

    fputslist(
        thisObject->fpAsm,
        // goto labelSymbol
        "@", labelSymbol, "\n",
        "0;JMP\n",
        NULL
    );
}

void CodeWriter_writeIf(CodeWriter thisObject, char *label)
{
    char labelSymbol[LABEL_SYMBOL_MAX_LENGTH + 1];
    sprintf(labelSymbol, "%s$%s", thisObject->vmFileName, label);

    fputslist(
        thisObject->fpAsm,
        // Memory[SP] -= 1
        "@SP\n",
        "M=M-1\n",
        // Register <- Memory[Memory[SP]]
        "@SP\n",
        "A=M\n",
        "D=M\n",
        // if jump(Register != 0) then goto labelSymbol
        "@", labelSymbol, "\n",
        "D;JNE\n",
        NULL
    );
}

void CodeWriter_writeCall(CodeWriter thisObject, char *functionName, int numArgs)
{
    char numArgsString[255];
    sprintf(numArgsString, "%d", numArgs);

    char returnLabel[255];
    sprintf(returnLabel, "%s$%d", functionName, thisObject->callCount);

    fputslist(
        thisObject->fpAsm,
        "// call ", functionName, " ", numArgsString, "\n",
        // Memory[Memory[SP]] <- return-address
        "@", returnLabel, "\n",
        "D=A\n",
        "@SP\n",
        "A=M\n",
        "M=D\n",
        // Memory[SP] += 1
        "@SP\n",
        "M=M+1\n",
        NULL
    );

    // push LCL, ARG, THIS, THAT
    writePushRegisterByName(thisObject->fpAsm, "LCL");
    writePushRegisterByName(thisObject->fpAsm, "ARG");
    writePushRegisterByName(thisObject->fpAsm, "THIS");
    writePushRegisterByName(thisObject->fpAsm, "THAT");

    fputslist(
        thisObject->fpAsm,
        // Memory[ARG] <- Memory[SP]-numArgsString-5
        "@SP\n",
        "D=M\n",
        "@", numArgsString, "\n",
        "D=D-A\n",
        "@5\n",
        "D=D-A\n",
        "@ARG\n",
        "M=D\n",
        // Memory[LCL] <- Memory[SP]
        "@SP\n",
        "D=M\n",
        "@LCL\n",
        "M=D\n",
        // goto function
        "@", functionName, "\n",
        "0;JMP\n",
        // (return-address) <- {thisObject->vmFileName}${thisObject->callCount}
        "(", returnLabel, ")\n",
        NULL
    );

    thisObject->callCount++;
}

void CodeWriter_writeReturn(CodeWriter thisObject)
{
    fputslist(
        thisObject->fpAsm,
        "// return\n",
        // Memory[R13] <- Memory[LCL]
        "@LCL\n",
        "D=M\n",
        "@R13\n",
        "M=D\n",
        // Memory[R14] <- Memory[Memory[R13]-5]
        "@5\n",
        "D=A\n",
        "@R13\n",
        "A=M-D\n",
        "D=M\n",
        "@R14\n",
        "M=D\n",
        // Memory[SP] -= 1
        "@SP\n",
        "M=M-1\n",
        // Memory[Memory[ARG]] <- Memory[Memory[SP]]
        "@SP\n",
        "A=M\n",
        "D=M\n",
        "@ARG\n",
        "A=M\n",
        "M=D\n",
        // Memory[SP] <- Memory[ARG] + 1
        "@ARG\n",
        "D=M+1\n",
        "@SP\n",
        "M=D\n",
        // Memory[THAT] <- Memory[Memory[R13]-1]
        "@1\n",
        "D=A\n",
        "@R13\n",
        "A=M-D\n",
        "D=M\n",
        "@THAT\n",
        "M=D\n",
        // Memory[THIS] <- Memory[Memory[R13]-2]
        "@2\n",
        "D=A\n",
        "@R13\n",
        "A=M-D\n",
        "D=M\n",
        "@THIS\n",
        "M=D\n",
        // Memory[ARG] <- Memory[Memory[R13]-3]
        "@3\n",
        "D=A\n",
        "@R13\n",
        "A=M-D\n",
        "D=M\n",
        "@ARG\n",
        "M=D\n",
        // Memory[LCL] <- Memory[Memory[R13]-4]
        "@4\n",
        "D=A\n",
        "@R13\n",
        "A=M-D\n",
        "D=M\n",
        "@LCL\n",
        "M=D\n",
        // goto Memory[R14]
        "@R14\n",
        "A=M\n",
        "0;JMP\n",
        NULL
    );
}

void CodeWriter_writeFunction(CodeWriter thisObject, char *functionName, int numLocals)
{
    char numLocalsString[255];
    sprintf(numLocalsString, "%d", numLocals);

    fputslist(
        thisObject->fpAsm,
        "// function ", functionName, " ", numLocalsString, "\n",
        "(", functionName, ")\n",
        NULL
    );
    for (int i = 0; i < numLocals; i++) {
        writePushConstant(thisObject->fpAsm, 0);
    }
}

void CodeWriter_close(CodeWriter thisObject)
{
    fclose(thisObject->fpAsm);
}

void writeArithmethicEqNext(CodeWriter thisObject)
{
    char skipLabel[ARITHMETIC_SKIP_LABEL_MAX_LENGTH + 1];
    sprintf(skipLabel, "%s.SKIP_EQ.%d", thisObject->vmFileName, thisObject->arithmeticEqCount);
    writeArithmethicEq(thisObject->fpAsm, skipLabel);
    thisObject->arithmeticEqCount++;
}

void writeArithmethicGtNext(CodeWriter thisObject)
{
    char skipLabel[ARITHMETIC_SKIP_LABEL_MAX_LENGTH + 1];
    sprintf(skipLabel, "%s.SKIP_GT.%d", thisObject->vmFileName, thisObject->arithmeticGtCount);
    writeArithmethicGt(thisObject->fpAsm, skipLabel);
    thisObject->arithmeticGtCount++;
}

void writeArithmethicLtNext(CodeWriter thisObject)
{
    char skipLabel[ARITHMETIC_SKIP_LABEL_MAX_LENGTH + 1];
    sprintf(skipLabel, "%s.SKIP_LT.%d", thisObject->vmFileName, thisObject->arithmeticLtCount);
    writeArithmethicLt(thisObject->fpAsm, skipLabel);
    thisObject->arithmeticLtCount++;
}

void writePushRegisterByName(FILE* fpAsm, char *registerName)
{
    fputslist(
        fpAsm,
        // Memory[Memory[SP]] <- Memory[registerName]
        "@", registerName, "\n",
        "D=M\n",
        "@SP\n",
        "A=M\n",
        "M=D\n",
        // Memory[SP] += 1
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}

// pop y, pop x, push (x + y)
void writeArithmethicAdd(FILE* fpAsm) { writeArithmethicBinaryOperation(fpAsm, "D+M"); }
// pop y, pop x, push (x - y)
void writeArithmethicSub(FILE* fpAsm) { writeArithmethicBinaryOperation(fpAsm, "M-D"); }
// pop y, push (-y)
void writeArithmethicNeg(FILE* fpAsm) { writeArithmethicUnaryOperation(fpAsm, "-M"); }

// pop y, pop x, push (x == y ? -1(true/0xFFFF) : 0(false/0x0000))
void writeArithmethicEq(FILE* fpAsm, char *skipLabel) { writeArithmethicCondition(fpAsm, skipLabel, "JEQ"); }
// pop y, pop x, push (x >  y ? -1(true/0xFFFF) : 0(false/0x0000))
void writeArithmethicGt(FILE* fpAsm, char *skipLabel) { writeArithmethicCondition(fpAsm, skipLabel, "JGT"); }
// pop y, pop x, push (x <  y ? -1(true/0xFFFF) : 0(false/0x0000))
void writeArithmethicLt(FILE* fpAsm, char *skipLabel) { writeArithmethicCondition(fpAsm, skipLabel, "JLT"); }

// pop y, pop x, push (x and y)
void writeArithmethicAnd(FILE* fpAsm) { writeArithmethicBinaryOperation(fpAsm, "D&M"); }
// pop y, pop x, push (x or y)
void writeArithmethicOr(FILE* fpAsm)  { writeArithmethicBinaryOperation(fpAsm, "D|M"); }
// pop y, push (not y)
void writeArithmethicNot(FILE* fpAsm) { writeArithmethicUnaryOperation(fpAsm, "!M"); }

// push index
void writePushConstant(FILE* fpAsm, int index)
{
    char indexStr[PUSH_POP_INDEX_MAX_DIGIT + 1];
    sprintf(indexStr, "%d", index);

    fputslist(
        fpAsm,
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

// push Memory[Memory[LCL]+index]
void writePushLocal(FILE* fpAsm, int index) { writePushSymbol(fpAsm, "LCL", index); }
// pop Memory[Memory[LCL]+index]
void writePopLocal(FILE* fpAsm, int index)  { writePopSymbol(fpAsm, "LCL", index); }

// push Memory[Memory[ARG]+index]
void writePushArgument(FILE* fpAsm, int index) { writePushSymbol(fpAsm, "ARG", index); }
// pop Memory[Memory[ARG]+index]
void writePopArgument(FILE* fpAsm, int index)  { writePopSymbol(fpAsm, "ARG", index); }

// push Memory[Memory[THIS]+index]
void writePushThis(FILE* fpAsm, int index) { writePushSymbol(fpAsm, "THIS", index); }
// pop Memory[Memory[THIS]+index]
void writePopThis(FILE* fpAsm, int index)  { writePopSymbol(fpAsm, "THIS", index); }

// push Memory[Memory[THAT]+index]
void writePushThat(FILE* fpAsm, int index) { writePushSymbol(fpAsm, "THAT", index); }
// pop Memory[Memory[THAT]+index]
void writePopThat(FILE* fpAsm, int index)  { writePopSymbol(fpAsm, "THAT", index); }

// push R{3+index}
void writePushPointer(FILE* fpAsm, int index) { writePushRegister(fpAsm, 3 + index); }
// pop R{3+index}
void writePopPointer(FILE* fpAsm, int index)  { writePopRegister(fpAsm, 3 + index); }

// push R{5+index}
void writePushTemp(FILE* fpAsm, int index) { writePushRegister(fpAsm, 5 + index); }
// pop R{5+index}
void writePopTemp(FILE* fpAsm, int index)  { writePopRegister(fpAsm, 5 + index); }

// push Memory[vmFileName.index]
void writePushStatic(FILE* fpAsm, char *vmFileName, int index)
{
    char symbol[PUSH_POP_SYMBOL_MAX_LENGTH + 1];
    sprintf(symbol, "%s.%d", vmFileName, index);

    fputslist(
        fpAsm,
        // Memory[Memory[SP]] <- Memory[symbol]
        "@", symbol, "\n",
        "D=M\n",
        "@SP\n",
        "A=M\n",
        "M=D\n",
        // Memory[SP] += 1
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}

// pop Memory[vmFileName.index]
void writePopStatic(FILE* fpAsm, char *vmFileName, int index)
{
    char symbol[PUSH_POP_SYMBOL_MAX_LENGTH + 1];
    sprintf(symbol, "%s.%d", vmFileName, index);

    fputslist(
        fpAsm,
        // Memory[SP] -= 1
        "@SP\n",
        "M=M-1\n",
        // Memory[symbol] <- Memory[Memory[SP]]
        "@SP\n",
        "A=M\n",
        "D=M\n",
        "@", symbol, "\n",
        "M=D\n",
        NULL
    );
}

// Unary operation (M <- y)
void writeArithmethicUnaryOperation(FILE* fpAsm, char *comp)
{
    fputslist(
        fpAsm,
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

// Binary operation (M <- x, D <- y)
void writeArithmethicBinaryOperation(FILE* fpAsm, char *comp)
{
    fputslist(
        fpAsm,
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
void writeArithmethicCondition(FILE* fpAsm, char *skipLabel, char *jump)
{
    fputslist(
        fpAsm,
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

// push Memory[Memory[Symbol]+index]
void writePushSymbol(FILE* fpAsm, char *symbol, int index)
{
    char indexStr[PUSH_POP_INDEX_MAX_DIGIT + 1];
    sprintf(indexStr, "%d", index);

    fputslist(
        fpAsm,
        "// push symbol ", symbol, " ", indexStr, "\n",
        // R13 <- Memory[Symbol]+index
        "@", indexStr, "\n",
        "D=A\n",
        "@", symbol, "\n",
        "D=D+M\n",
        "@R13\n",
        "M=D\n",
        // Memory[Memory[SP]] <- Memory[R13] 
        "A=M\n",
        "D=M\n",
        "@SP\n",
        "A=M\n",
        "M=D\n",
        // Memory[SP] += 1
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}

// pop Memory[Memory[Symbol]+index]
void writePopSymbol(FILE* fpAsm, char *symbol, int index)
{
    char indexStr[PUSH_POP_INDEX_MAX_DIGIT + 1];
    sprintf(indexStr, "%d", index);

    fputslist(
        fpAsm,
        // Memory[SP] -= 1
        "@SP\n",
        "M=M-1\n",
        // R13 <- Memory[Symbol]+index
        "@", indexStr, "\n",
        "D=A\n",
        "@", symbol, "\n",
        "D=D+M\n",
        "@R13\n",
        "M=D\n",
        // Memory[R13] <- Memory[Memory[SP]]
        "@SP\n",
        "A=M\n",
        "D=M\n",
        "@R13\n",
        "A=M\n",
        "M=D\n",
        NULL
    );
}

// push R{registerNumber}
void writePushRegister(FILE* fpAsm, int registerNumber)
{
    char symbol[8];
    sprintf(symbol, "R%d", registerNumber);

    fputslist(
        fpAsm,
        // Memory[Memory[SP]] <- register
        "@", symbol, "\n",
        "D=M\n",
        "@SP\n",
        "A=M\n",
        "M=D\n",
        // Memory[SP] += 1
        "@SP\n",
        "M=M+1\n",
        NULL
    );
}

// pop R{registerNumber}
void writePopRegister(FILE* fpAsm, int registerNumber)
{
    char symbol[8];
    sprintf(symbol, "R%d", registerNumber);

    fputslist(
        fpAsm,
        // Memory[SP] -= 1
        "@SP\n",
        "M=M-1\n",
        // register <- Memory[Memory[SP]]
        "@SP\n",
        "A=M\n",
        "D=M\n",
        "@", symbol, "\n",
        "M=D\n",
        NULL
    );
}

void fputslist(FILE* fp, ...)
{
    char* string;

    va_list args;
    va_start(args, fp);

    while ((string = va_arg(args, char*)) != NULL) {
        fputs(string, fp);
    }

    va_end(args);
}