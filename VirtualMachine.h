#pragma once

#define INSN_TYPE   char
#define INSN_SIZE   (sizeof(char))

#define DATA_TYPE   int
#define DATA_SIZE   (sizeof(int))

#include <stack>
#include <vector>
#include <string>

using namespace std;

class VirtualMachine {
public:
    VirtualMachine() = default;
    ~VirtualMachine() {
        stop();
    }

protected:
    char running = 0;

    std::stack<DATA_TYPE> stck;
    DATA_TYPE pc = 0;
    DATA_TYPE reg = 0;

    enum OpCode {
        HALT,
        PUSH,
        POP,
        EQU,
        NEQU,
        GT,
        LT,
        ADD,
        SUB,
        MUL,
        DIV,
        JMP,
        JMPZ,
        PRINT,
        NOP
    };

    struct Instruction {
        OpCode opcode;
        std::string sz;
        char has_arg;
    };

    #define INS(o,a) { o, #o, a }
    std::vector<Instruction> Instructions = {
        INS(PUSH, 1),
        INS(POP, 0),
        
        INS(EQU, 0),
        INS(NEQU, 0),
        INS(GT, 0),
        INS(LT, 0),


        INS(ADD, 0),
        INS(SUB, 0),
        INS(MUL, 0),
        INS(DIV, 0),
        INS(JMP, 1),
        INS(JMPZ, 1),
        INS(PRINT, 0),

        INS(HALT, 0),
        INS(NOP, 0),
    };
    #undef INS

    vector<char> program = {};

public:
    void compile(std::string path, std::string output);
    void decompile(std::string path, std::string output);
    void execute(INSN_TYPE opcode);
    void save(std::string path);
    void load(std::string path);

    void run();
    void stop();
};