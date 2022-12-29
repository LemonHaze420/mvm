#pragma once

#define INSN_TYPE   char
#define INSN_SIZE   (sizeof(char))

#define DATA_TYPE   unsigned short
#define DATA_SIZE   (sizeof(unsigned short))

#define CATCH               catch (std::exception& e) {                                          \
                                cerr << "An exception occurred!\n\n";                             \
                                cerr << e.what() << endl;                                         \
                                cerr << "PC: 0x" << std::hex << pc << endl;                       \
                                cerr << "R0: 0x" << std::hex << reg << endl;                      \
                                cerr << "program size: 0x" << std::hex << program.size() << endl; \
                           }                                                                      \

// disable logging on Release
#ifndef _DEBUG
#define LOG        / ## /
#else
#define LOG        cout
#endif

#include <iostream>
#include <stack>
#include <vector>
#include <string>

using namespace std;

enum OpCode {
    CALL,

    NOP,

    PUSH,
    POP,

    LOAD,

    EQU,
    NEQU,
    GT,
    GTEQ,
    LT,
    LTEQ,

    ADD,
    SUB,
    MUL,
    DIV,

    XOR,
    OR,
    MOD,
    NEG,
    AND,

    JMP,
    JMPZ,
    JMPNZ,

    PRINT,

    HALT,
};

struct Instruction {
    OpCode opcode;
    std::string sz;
    char num_args;
};

class mvm {
public:
    mvm() = default;
    ~mvm() {
        stop();
    }

protected:
    char running = 0;

    std::stack<DATA_TYPE> stck;
    DATA_TYPE pc = 0;
    DATA_TYPE reg = 0;

    #define INS(o,a) { o, #o, a }
    std::vector<Instruction> instruction_definitions = {
        INS(CALL, 1),

        INS(NOP, 0),

        INS(PUSH, 1),
        INS(POP, 0),
        
        INS(LOAD, 0),

        INS(EQU, 0),
        INS(NEQU, 0),
        INS(GT, 0),
        INS(GTEQ, 0),
        INS(LT, 0),
        INS(LTEQ, 0),

        INS(ADD, 0),
        INS(SUB, 0),
        INS(MUL, 0),
        INS(DIV, 0),

        INS(XOR,0),
        INS(OR, 0),
        INS(NEG,0),
        INS(AND,0),

        INS(JMP, 1),
        INS(JMPZ, 1),
        INS(JMPNZ, 1),

        INS(PRINT, 0),

        INS(HALT, 0),
    };
    #undef INS

    vector<char> program = {};

public:
    bool compile(std::string path, std::string output);
    void translate_to_x64_asm(std::string path, std::string output);
    bool decompile(std::string path, std::string output);
    void execute(INSN_TYPE opcode);
    bool save(std::string path);
    bool load(std::string path);

    void start();
    void stop();
};