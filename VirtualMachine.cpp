#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>

#include "VirtualMachine.h"

void VirtualMachine::compile(std::string path, std::string output) {
    if (path.empty() || output.empty())
        return;

    std::ifstream source(path);
    std::ofstream binary(output, std::ios::binary);
    if (!source.good() || !binary.good())
        return;

    string line;
    while (getline(source, line)) {
        if (line.empty() || line[0] == '#') // ignore empty lines and ignore lines starting with # for comments
            continue;         
        
        string opcode_str;
        DATA_TYPE arg = 0;
        size_t space_pos = line.find(' ');
        if (space_pos == string::npos)
            opcode_str = line;
        else {
            opcode_str = line.substr(0, space_pos);
            arg = stoi(line.substr(space_pos + 1));
        }

        bool found = false;
        for (auto& insn : Instructions) {
            if (opcode_str == insn.sz) {
                binary.write(reinterpret_cast<char*>(&insn.opcode), INSN_SIZE);
                if (insn.has_arg)
                    binary.write(reinterpret_cast<char*>(&arg), DATA_SIZE);
                found = true;
                break;
            }
        }

        if (!found) {
            cout << "Invalid opcode: " << opcode_str << endl;
            return;
        }
    }
    printf("compiled '%s'\n", path.c_str());
}
void VirtualMachine::decompile(std::string path, std::string output) {
    if (path.empty() || output.empty())
        return;

    std::ifstream in(path);
    std::ofstream out(output, std::ios::binary);
    if (!in.good() || !out.good())
        return;

    INSN_TYPE instr;
    while (in.read(reinterpret_cast<char*>(&instr), INSN_SIZE)) {
        bool found = false;
        for (auto& insn : Instructions) {
            if (instr == insn.opcode) {
                out << insn.sz;
                if (insn.has_arg) {
                    DATA_TYPE arg;
                    in.read(reinterpret_cast<char*>(&arg), DATA_SIZE);
                    out << " " << arg << endl;
                } else
                    out << endl;
                found = true;
                break;
            }
        }
        if (!found)
            out << "INVALID OPCODE" << endl;
    }
    printf("decompiled '%s'\n", path.c_str());
}
void VirtualMachine::execute(INSN_TYPE opcode) {
    switch (opcode) {
        case NOP: {
            break;
        }
        case HALT: {
            cout << "HALT" << endl;
            running = 0;
            exit(0);
        }
        case PUSH: {
            pc += INSN_SIZE;
            DATA_TYPE value = program.at(pc);
            stck.push(value);
            pc += DATA_SIZE - INSN_SIZE;
            break;
        }
        case POP: {
            reg = stck.top();
            stck.pop();
            break;
        }
        case EQU: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(a == b ? 1 : 0);
            break;
        }
        case NEQU: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(a != b ? 1 : 0);
            break;
        }
        case GT: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(a >= b ? 1 : 0);
            break;
        }
        case LT: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(a <= b ? 1 : 0);
            break;
        }
        case ADD: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(a + b);
            break;
        }
        case SUB: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(b - a);
            break;
        }
        case MUL: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(a * b);
            break;
        }
        case DIV: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(b / a);
            break;
        }
        case JMP: {
            DATA_TYPE target = program.at(pc);
            pc = target - 1;
            pc += DATA_SIZE - INSN_SIZE;
            break;
        }
        case JMPZ: {
            pc += INSN_SIZE;
            DATA_TYPE target = program.at(pc);
            if (stck.top() == 0)
                pc = target - 1;
            pc += DATA_SIZE - INSN_SIZE;
            break;
        }
        case PRINT: {
            cout << "PRINT " << stck.top() << endl;
            break;
        }
        default: {
            cout << "Invalid opcode: " << opcode << endl;
            exit(1);
        }
    }
}
void VirtualMachine::save(std::string path) {
    if (!program.size())
        return;
    if (path.empty())
        return;

    std::ofstream out(path, std::ios::binary);
    if (!out.good())
        return;
    out.write(reinterpret_cast<char*>(&program.at(0)), program.size());
    out.close();
}
void VirtualMachine::load(std::string path) {
    if (path.empty())
        return;

    std::ifstream in(path, std::ios::binary);
    if (!in.good())
        return;

    in.seekg(0, std::ios::end);
    size_t sz = in.tellg();
    in.seekg(0, std::ios::beg);

    program.clear();
    program.resize(sz);

    in.read(reinterpret_cast<char*>(&program.at(0)), sz);
    in.close();
}
void VirtualMachine::run() {
    printf("running program...\n\n");
    running = 1;

    while (running) {
        execute(program.at(pc));
        pc += INSN_SIZE;
    }
}
void VirtualMachine::stop() {
    running = 0;
}