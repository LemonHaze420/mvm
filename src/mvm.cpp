#include "mvm.h"

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <fstream>
#include <cstdint>
#include <unordered_map>

bool mvm::compile(std::string path, std::string output) {
    if (path.empty() || output.empty())
        return false;

    std::ifstream source(path);
    std::ofstream binary(output, std::ios::binary);
    if (!source.good() || !binary.good())
        return false;

    // First pass: build list of label names and their locations
    std::unordered_map<std::string, DATA_TYPE> label_offsets;
    size_t bytecode_size = 0;
    string line;
    while (getline(source, line)) {
        if (line.empty() || line[0] == '#' || line.find("#") != std::string::npos) 
            continue;

        if (line[line.size() - 1] == ':') {
            // This is a label definition
            std::string label_name = line.substr(0, line.size() - 1);
            if (label_offsets.count(label_name) > 0) {
                cerr << "Duplicate label name: " << label_name << endl;
                return false;
            }
            label_offsets[label_name] = (DATA_TYPE)bytecode_size;
        }
        else {
            // This is an instruction
            string opcode_str;
            size_t space_pos = line.find(' ');
            if (space_pos == string::npos)
                opcode_str = line;
            else
                opcode_str = line.substr(0, space_pos);

            bool found = false;
            for (auto& insn : instruction_definitions) {
                if (opcode_str == insn.sz) {
                    bytecode_size += INSN_SIZE + DATA_SIZE * insn.num_args;
                    found = true;
                    break;
                }
            }

            if (!found) {
                cerr << "Invalid opcode: " << opcode_str << endl;
                return false;
            }
        }
    }

    // Second pass: generate bytecode
    source.clear();
    source.seekg(0);
    while (getline(source, line)) {
        if (line.empty() || line[0] == '#' || line.find("#") != std::string::npos)
            continue;

        if (line[line.size() - 1] == ':') { // Label definition
            string label_name = line.substr(0, line.size() - 1);
            label_offsets[label_name] = (DATA_TYPE)binary.tellp();
            continue;
        }

        string opcode_str;
        DATA_TYPE arg = 0;
        size_t space_pos = line.find(' ');
        if (space_pos == string::npos)
            opcode_str = line;
        else {
            opcode_str = line.substr(0, space_pos);
            if (isdigit(line[space_pos + 1]))
                arg = stoi(line.substr(space_pos + 1));
            else { // Label reference
                string label_name = line.substr(space_pos + 1);
                if (label_offsets.find(label_name) == label_offsets.end()) {
                    cerr << "Invalid label name: " << label_name << endl;
                    return false;
                }
                arg = label_offsets[label_name];
            }
        }

        bool found = false;
        for (auto& insn : instruction_definitions) {
            if (opcode_str == insn.sz) {
                binary.write(reinterpret_cast<char*>(&insn.opcode), INSN_SIZE);
                binary.write(reinterpret_cast<char*>(&arg), DATA_SIZE * insn.num_args);
                found = true;
                break;
            }
        }

        if (!found) {
            cerr << "Invalid opcode: " << opcode_str << endl;
            return false;
        }
    }
    return true;
}
bool mvm::decompile(std::string path, std::string output) {
    if (path.empty() || output.empty())
        return false;

    std::ifstream in(path);
    std::ofstream out(output, std::ios::binary);
    if (!in.good() || !out.good())
        return false;

    INSN_TYPE instr;
    while (in.read(reinterpret_cast<char*>(&instr), INSN_SIZE)) {
        bool found = false;
        for (auto& insn : instruction_definitions) {
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
            out << "INVALID OPCODE\n";
    }
    return true;
}
void mvm::translate_to_x64_asm(std::string path, std::string output) {
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
            case LOAD:
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
            case JMP:
            case JMPNZ:
            case JMPZ: {
                READ();
                if (instr == JMP) {
                    out << "    jmp $0x" << std::hex << value << endl;
                } else {
                    out << "    pop %rax\n";
                    out << "    cmp $0, %rax\n";
                    if (instr == JMPNZ)
                        out << "    jne $0x" << std::hex << value << endl;
                    else
                        out << "    je $0x" << std::hex << value << endl;
                }
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
                out << "    add $0x30, %rax\n"; // shift into ASCII integer characters
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
                out << "    mov $1, %rax\n";
                out << "    mov $1, %rdi\n";
                out << "    mov %rsp, %rsi\n";
                out << "    mov $1, %rdx\n";
                out << "    syscall\n";
                out << "    add $8, %rsp\n";
                break;
            }
            case CALL: {
                READ();
                break;
            }
            case NOP: {
                out << "    nop\n";
                break;
            }
        }
    }
    #undef READ

    // Prepare the data segment
    out << ".section .rodata\n";
    out << "\n";

    out << "\n";
}

void __sleep(DATA_TYPE val) {
#ifdef _MSC_VER
    Sleep(*reinterpret_cast<int*>(&val));
#else
    sleep(*reinterpret_cast<int*>(&val));
#endif
}

void (*func_table[])(DATA_TYPE val) = {
    __sleep
};

void mvm::execute(INSN_TYPE opcode) {
    try {
        DATA_TYPE immediate = 0;
        switch (opcode) {
            case CALL: {
                pc += INSN_SIZE;
                immediate = *reinterpret_cast<DATA_TYPE*>(&program.at(pc));
                func_table[immediate](stck.top());
                stck.pop();
                pc += DATA_SIZE - INSN_SIZE;
                break;
            }
            case LOAD: {
                stck.push(reg);
                break;
            }
            case NOP: {
                break;
            }
            case HALT: {
                LOG << "HALT\n";
                running = 0;
    #           ifndef _DEBUG
                    exit(0);
    #           endif
                break;
            }
            case PUSH: {
                pc += INSN_SIZE;
                immediate = *reinterpret_cast<DATA_TYPE*>(&program.at(pc));
                stck.push(immediate);
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
            case JMP:
            case JMPZ:
            case JMPNZ: {
                pc += INSN_SIZE;
                if (opcode == JMP) {
                    immediate = *reinterpret_cast<DATA_TYPE*>(&program.at(pc)) - 1;
                    pc = immediate - 1;
                } else if (opcode == JMPZ ? stck.top() == 0 : stck.top() != 0) {
                    immediate = *reinterpret_cast<DATA_TYPE*>(&program.at(pc)) - 1;
                    pc = immediate - 1;
                    stck.pop();
                }
                pc += DATA_SIZE - INSN_SIZE;
                break;
            }
            case PRINT: {
                cout << "PRINT " << stck.top() << endl;
                break;
            }
            default: {
                throw new std::runtime_error("Invalid opcode " + opcode);
            }
        }
    }
    CATCH
}

bool mvm::save(std::string path) {
    if (!program.size() || path.empty())
        return false;

    std::ofstream out(path, std::ios::binary);
    if (!out.good())
        return false;

    out.write(reinterpret_cast<char*>(&program.at(0)), program.size());
    out.close();
    return true;
}
bool mvm::load(std::string path) {
    if (path.empty())
        return false;

    std::ifstream in(path, std::ios::binary);
    if (!in.good())
        return false;

    in.seekg(0, std::ios::end);
    size_t sz = in.tellg();
    in.seekg(0, std::ios::beg);
    if (!sz) {
        in.close();
        return false;
    }

    program.clear();
    program.resize(sz);

    in.read(reinterpret_cast<char*>(&program.at(0)), sz);
    in.close();
    return true;
}


void mvm::start() {
    running = 1;

    try {
        while (running && pc < program.size()) {
            execute(program.at(pc));

            pc += INSN_SIZE;
        }
    } 
    CATCH
}
void mvm::stop() {
    running = 0;
}