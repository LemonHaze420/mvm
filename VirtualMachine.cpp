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
        if (line.empty()) continue;         // ignore empty lines
        if (line[0] == '#') continue;       // ignore lines starting with # for comments

        string opcode_str;
        INSN_TYPE arg = 0;
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
                INSN_TYPE instr[2];
                instr[0] = insn.opcode;
                instr[1] = arg;
                binary.write(reinterpret_cast<char*>(&instr), INSN_SIZE * (insn.has_arg ? 2 : 1));
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

    INSN_TYPE instr[2];
    while (in.read(reinterpret_cast<char*>(&instr), INSN_SIZE)) {
        bool found = false;
        for (auto& insn : Instructions) {
            if (instr[0] == insn.opcode) {
                out << insn.sz;
                if (insn.has_arg) {
                    in.read(reinterpret_cast<char*>(&instr[1]), INSN_SIZE);
                    out << " " << instr[1] << endl;
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
            INSN_TYPE value = program.at(++pc);
            stck.push(value);
            break;
        }
        case POP: {
            reg = stck.top();
            stck.pop();
            break;
        }
        case EQU: {
            INSN_TYPE a = stck.top();
            stck.pop();
            INSN_TYPE b = stck.top();
            stck.pop();
            stck.push(a == b ? 1 : 0);
            break;
        }
        case NEQU: {
            INSN_TYPE a = stck.top();
            stck.pop();
            INSN_TYPE b = stck.top();
            stck.pop();
            stck.push(a != b ? 1 : 0);
            break;
        }
        case GT: {
            INSN_TYPE a = stck.top();
            stck.pop();
            INSN_TYPE b = stck.top();
            stck.pop();
            stck.push(a >= b ? 1 : 0);
            break;
        }
        case LT: {
            INSN_TYPE a = stck.top();
            stck.pop();
            INSN_TYPE b = stck.top();
            stck.pop();
            stck.push(a <= b ? 1 : 0);
            break;
        }
        case ADD: {
            INSN_TYPE a = stck.top();
            stck.pop();
            INSN_TYPE b = stck.top();
            stck.pop();
            stck.push(a + b);
            break;
        }
        case SUB: {
            INSN_TYPE a = stck.top();
            stck.pop();
            INSN_TYPE b = stck.top();
            stck.pop();
            stck.push(b - a);
            break;
        }
        case MUL: {
            INSN_TYPE a = stck.top();
            stck.pop();
            INSN_TYPE b = stck.top();
            stck.pop();
            stck.push(a * b);
            break;
        }
        case DIV: {
            INSN_TYPE a = stck.top();
            stck.pop();
            INSN_TYPE b = stck.top();
            stck.pop();
            stck.push(b / a);
            break;
        }
        case JMP: {
            INSN_TYPE target = program.at(++pc);
            pc = target - 1;
            break;
        }
        case JMPZ: {
            INSN_TYPE target = program.at(++pc);
            if (stck.top() == 0) {
                pc = target - 1;
            }
            break;
        }
        case PRINT: {
            cout << stck.top() << endl;
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
    out.write(reinterpret_cast<char*>(&program.at(0)), program.size() * INSN_SIZE);
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
    program.resize(sz / INSN_SIZE);

    in.read(reinterpret_cast<char*>(&program.at(0)), sz);
    in.close();

    program.shrink_to_fit();
}
void VirtualMachine::run() {
    printf("running program...\n\n");
    running = 1;

    while (running) {
        INSN_TYPE opcode = program.at(pc);
        execute(opcode);
        pc++;
    }
}
void VirtualMachine::stop() {
    running = 0;
}