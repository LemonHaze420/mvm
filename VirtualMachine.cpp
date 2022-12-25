#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>

#include "VirtualMachine.h"

void VirtualMachine::compile(std::string path, std::string output) {
    if (path.empty())
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
        int32_t arg = 0;
        size_t space_pos = line.find(' ');
        if (space_pos == string::npos)
            opcode_str = line;
        else {
            opcode_str = line.substr(0, space_pos);
            arg = stoi(line.substr(space_pos + 1));
        }

        Instruction instr;
        instr.arg = arg;
        bool hasArg = false;
        if (opcode_str == "PUSH") {
            hasArg = true;
            instr.opcode = PUSH;
        }
        else if (opcode_str == "POP") {
            instr.opcode = POP;
        }
        else if (opcode_str == "EQU") {
            instr.opcode = EQU;
        }
        else if (opcode_str == "NEQU") {
            instr.opcode = NEQU;
        }
        else if (opcode_str == "ADD") {
            instr.opcode = ADD;
        }
        else if (opcode_str == "SUB") {
            instr.opcode = SUB;
        }
        else if (opcode_str == "MUL") {
            instr.opcode = MUL;
        }
        else if (opcode_str == "DIV") {
            instr.opcode = DIV;
        }
        else if (opcode_str == "JMP") {
            hasArg = true;
            instr.opcode = JMP;
        }
        else if (opcode_str == "JMPZ") {
            hasArg = true;
            instr.opcode = JMPZ;
        }
        else if (opcode_str == "PRINT") {
            instr.opcode = PRINT;
        }
        else if (opcode_str == "HALT") {
            instr.opcode = HALT;
        }
        else {
            cout << "Invalid opcode: " << opcode_str << endl;
            return;
        }

        if (hasArg)
            binary.write(reinterpret_cast<char*>(&instr), sizeof(Instruction));
        else
            binary.write(reinterpret_cast<char*>(&instr), 4);
    }
}
void VirtualMachine::decompile(std::string path, std::string output) {
    if (path.empty() || output.empty())
        return;

    std::ifstream in(path);
    std::ofstream out(output, std::ios::binary);
    if (!in.good() || !out.good())
        return;

    Instruction instr;
    while (in.read(reinterpret_cast<char*>(&instr), 4)) {
        switch (instr.opcode) {
        case PUSH:
            in.read(reinterpret_cast<char*>(&instr.arg), 4);
            out << "PUSH " << instr.arg << endl;
            break;
        case POP:
            out << "POP" << endl;
            break;
        case EQU:
            out << "EQU" << endl;
            break;
        case NEQU:
            out << "NEQU" << endl;
            break;
        case ADD:
            out << "ADD" << endl;
            break;
        case SUB:
            out << "SUB" << endl;
            break;
        case MUL:
            out << "MUL" << endl;
            break;
        case DIV:
            out << "DIV" << endl;
            break;
        case JMP:
            in.read(reinterpret_cast<char*>(&instr.arg), 4);
            out << "JMP " << instr.arg << endl;
            break;
        case JMPZ:
            in.read(reinterpret_cast<char*>(&instr.arg), 4);
            out << "JMPZ " << instr.arg << endl;
            break;
        case PRINT:
            out << "PRINT" << endl;
            break;
        case HALT:
            out << "HALT" << endl;
            break;
        default:
            out << "INVALID OPCODE" << endl;
            break;
        }
    }
}
void VirtualMachine::execute(int opcode) {
    switch (opcode) {
    case HALT: {
        cout << "HALT" << endl;
        exit(0);
    }
    case PUSH: {
        int value = program.at(++pc);
        stck.push(value);
        break;
    }
    case POP: {
        reg = stck.top();
        stck.pop();
        break;
    }
    case EQU: {
        int a = stck.top();
        stck.pop();
        int b = stck.top();
        stck.pop();
        stck.push(a == b ? 1 : 0);
        break;
    }
    case NEQU: {
        int a = stck.top();
        stck.pop();
        int b = stck.top();
        stck.pop();
        stck.push(a != b ? 1 : 0);
        break;
    }
    case ADD: {
        int a = stck.top();
        stck.pop();
        int b = stck.top();
        stck.pop();
        stck.push(a + b);
        break;
    }
    case SUB: {
        int a = stck.top();
        stck.pop();
        int b = stck.top();
        stck.pop();
        stck.push(b - a);
        break;
    }
    case MUL: {
        int a = stck.top();
        stck.pop();
        int b = stck.top();
        stck.pop();
        stck.push(a * b);
        break;
    }
    case DIV: {
        int a = stck.top();
        stck.pop();
        int b = stck.top();
        stck.pop();
        stck.push(b / a);
        break;
    }
    case JMP: {
        int target = program.at(++pc);
        pc = target - 1;
        break;
    }
    case JMPZ: {
        int target = program.at(++pc);
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
    out.write(reinterpret_cast<char*>(&program.at(0)), program.size() * sizeof(int));
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
    program.resize(sz / 4);

    in.read(reinterpret_cast<char*>(&program.at(0)), sz);
    in.close();

    program.shrink_to_fit();
}
void VirtualMachine::run() {
    while (true) {
        int opcode = program.at(pc);
        execute(opcode);
        pc++;
    }
}