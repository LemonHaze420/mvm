#pragma once
#include <stack>
#include <vector>
#include <string>

using namespace std;

class VirtualMachine {
public:
    VirtualMachine() = default;
    ~VirtualMachine() = default;

protected:
    std::stack<int> stck;
    int pc = 0;
    int reg = 0;

    enum OpCode {
        HALT,
        PUSH,
        POP,
        EQU,
        NEQU,
        ADD,
        SUB,
        MUL,
        DIV,
        JMP,
        JMPZ,
        PRINT
    };

    struct Instruction {
        OpCode opcode;
        int arg;
    };

    vector<int> program = {
        HALT
    };

public:
    void compile(std::string path, std::string output);
    void decompile(std::string path, std::string output);
    void execute(int opcode);
    void save(std::string path);
    void load(std::string path);
    void run();
};