#include "VirtualMachine.h"

int main(int argc, char **argp) {
    if (argc < 2)
        return -1;

    VirtualMachine vm;
    if (strstr(argp[1], "-t")) {
        vm.compile("program.s", "program.bin");
        vm.translate_to_x64_asm("program.bin", "program.x64.s");
        vm.decompile("program.bin", "program.bin.dec");
        vm.load("program.bin");
        vm.start();
        return 1;
    }

    bool bRun = true;
    if (strstr(argp[1], "-c") || strstr(argp[1], "-d"))
        bRun = false;
    
    std::string in = argp[bRun ? 1 : 2];
    if (!bRun) {
        std::string out = argp[3];

        if (strstr(argp[1], "-c"))
            vm.compile(in, out);
        else
            vm.decompile(in, out);
    } else {
        vm.load(in);
        vm.start();
    }
    return 0;
}