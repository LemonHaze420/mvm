#include "VirtualMachine.h"

int main(int argc, char **argp) {
    if (argc < 2)
        return -1;

    bool bRun = true;
    if (strstr(argp[1], "-c") || strstr(argp[1], "-d"))
        bRun = false;
    
    VirtualMachine vm;
    std::string in = argp[bRun ? 1 : 2];
    if (!bRun) {
        std::string out = argp[3];

        if (strstr(argp[1], "-c"))
            vm.compile(in, out);
        else
            vm.decompile(in, out);
    } else {
        vm.load(in);
        vm.run();
    }
    return 0;
}