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
        if (line.empty() || line[0] == '#' || line.find("#") != std::string::npos) // ignore empty lines and ignore lines starting with (or containing) # for comments
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
                if (insn.num_args)
                    binary.write(reinterpret_cast<char*>(&arg), DATA_SIZE * insn.num_args);
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
                DATA_TYPE arg;
                for (int i = 0; i < insn.num_args; ++i) {
                    in.read(reinterpret_cast<char*>(&arg), DATA_SIZE);
                    out << " " << arg;
                }
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

void VirtualMachine::translate_to_x64_asm(std::string path, std::string output) {
    std::ifstream in(path, std::ios::binary);
    std::ofstream out(output);

    if (!in.good() || !out.good())
        return;

    out << ".section .text\n";
    out << ".globl _start\n";
    out << "_start:\n";

    INSN_TYPE instr;
    DATA_TYPE value;
    #define READ()  in.read(reinterpret_cast<char*>(&value), DATA_SIZE)
    while (in.read(reinterpret_cast<char*>(&instr), INSN_SIZE)) {
        switch (instr) {
            case PUSH:
                READ();
                out << "    movabs $" << value << ", %rax\n";
                out << "    push %rax\n";
                break;
            case POP:
                out << "    pop %rax\n";
                break;
            case ADD: {
                out << "    pop %rbx\n";
                out << "    pop %rax\n";
                out << "    add %rbx, %rax\n";
                out << "    push %rax\n";
                break;
            }
            case SUB:
                out << "    pop %rbx\n";
                out << "    pop %rax\n";
                out << "    sub %rax, %rbx\n";
                out << "    push %rbx\n";
                break;
            case XOR: {
                out << "    pop %rax\n";
                out << "    pop %rbx\n";
                out << "    xor %rax, %rbx\n";
                out << "    push %rax\n";
                break;
            }
            case AND: {
                out << "    pop %rax\n";
                out << "    pop %rbx\n";
                out << "    and %rax, %rbx\n";
                out << "    push %rax\n";
                break;
            }
            case OR: {
                out << "    pop %rax\n";
                out << "    pop %rbx\n";
                out << "    or %rax, %rbx\n";
                out << "    push %rax\n";
                break;
            }
            case NEG: {
                out << "    pop %rax\n";
                out << "    neg %rax\n";
                out << "    push %rax\n";
                break;
            }
            case MUL:
                out << "    pop %rax\n";
                out << "    pop %rcx\n";
                out << "    imul %rcx, %rax\n";
                out << "    push %rax\n";
                break;
            case DIV:
                out << "    pop %rax\n";
                out << "    pop %rcx\n";
                out << "    xor %rdx, %rdx\n";  // Clear rdx to zero
                out << "    div %rcx\n";
                out << "    push %rax\n";
                break;
            case MOD:
                out << "    pop %rax\n";
                out << "    pop %rcx\n";
                out << "    xor %rdx, %rdx\n";  // Clear rdx to zero
                out << "    div %rcx\n";
                out << "    push %rdx\n";
                break;
            case EQU:
                out << "    pop %rax\n";
                out << "    pop %rbx\n";
                out << "    cmp %rax, %rbx\n";
                out << "    sete %al\n";
                out << "    movsx %al, %rax\n";
                out << "    push %rax\n";
                break;
            case NEQU:
                out << "    pop %rax\n";
                out << "    pop %rbx\n";
                out << "    cmp %rax, %rbx\n";
                out << "    setne %al\n";
                out << "    movsx %al, %rax\n";
                out << "    push %rax\n";
                break;
            case LT: {
                out << "    pop %rbx\n";
                out << "    pop %rax\n";
                out << "    cmp %rax, %rbx\n";
                out << "    setl %al\n";
                out << "    and $1, %al\n";
                out << "    push %rax\n";
                break;
            }
            case LTEQ: {
                out << "    pop %rax\n";
                out << "    pop %rbx\n";
                out << "    cmp %bx, %ax\n";
                out << "    setle %al\n";
                out << "    movzbl %al, %eax\n";
                out << "    push %rax\n";
                break;
            }
            case GT:
                out << "    pop %rbx\n";
                out << "    pop %rax\n";
                out << "    cmp %rax, %rbx\n";
                out << "    setg %al\n";
                out << "    movzb %rax, %al\n";
                out << "    push %rax\n";
                break;
            case GTEQ:
                out << "    pop %rbx\n";
                out << "    pop %rax\n";
                out << "    cmp %rax, %rbx\n";
                out << "    setge %al\n";
                out << "    movzb %rax, %al\n";
                out << "    push %rax\n";
                break;
            case JMP: {
                READ();
                out << "    jmp .L" << value << endl;
                break;
            }
            case JMPZ: {
                READ();
                out << "    pop %rax\n";
                out << "    cmp $0, %rax\n";
                out << "    je .L" << value << endl;
                break;
            }
            case HALT: {
                out << "    mov $60, %rax\n";
                out << "    xor %rdi, %rdi\n";
                out << "    syscall\n";
                break;
            }
            case PRINT: {
                // print out the value
                out << "    pop %rax\n";
                out << "    push %rax\n";
                out << "    mov $1, %rax\n";
                out << "    mov $1, %rdi\n";
                out << "    mov %rsp, %rsi\n";
                out << "    mov $1, %rdx\n";
                out << "    syscall\n";
                out << "    add $8, %rsp\n";

                // print out a newline
                out << "    push $0x0d\n";
                out << "    mov $1, % rax\n";
                out << "    mov $1, % rdi\n";
                out << "    mov% rsp, % rsi\n";
                out << "    mov $1, % rdx\n";
                out << "    syscall\n";
                out << "    add $8, %rsp\n";
                break;
            }
            case NOP: {
                out << "    nop\n";
                break;
            }
        }
    }

    // Prepare the data segment
    out << ".section .rodata\n";
    out << "\n";

    out << "\n";
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
            stck.push(a > b ? 1 : 0);
            break;
        }
        case LT: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(a < b ? 1 : 0);
            break;
        }
        case GTEQ: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(a >= b ? 1 : 0);
            break;
        }
        case LTEQ: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(a <= b ? 1 : 0);
            break;
        }
        case XOR: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(a ^ b);
            break;
        }
        case OR: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(a | b);
            break;
        }
        case MOD: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(a % b);
            break;
        }
        case NEG: {
            DATA_TYPE v = ~stck.top();
            stck.pop();
            stck.push(v);
            break;
        }
        case AND: {
            DATA_TYPE a = stck.top();
            stck.pop();
            DATA_TYPE b = stck.top();
            stck.pop();
            stck.push(a & b);
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
            pc += INSN_SIZE;
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
void VirtualMachine::start() {
    running = 1;

    while (running) {
        execute(program.at(pc));
        pc += INSN_SIZE;
    }
}
void VirtualMachine::stop() {
    running = 0;
}