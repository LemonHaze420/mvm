#include "VirtualMachine.h"

int main(int argc, char **argp) {
    if (argc < 2)
        return -1;

    VirtualMachine vm;
    if (strstr(argp[1], "-t")) {
        vm.compile("scripts\\count_to_100.mvms", "bin\\count_to_100.mvmb");
        vm.translate_to_x64_asm("bin\\count_to_100.mvmb", "bin\\count_to_100.x64.s");
        vm.decompile("bin\\count_to_100.mvmb", "bin\\count_to_100.dec.mvms");
        if (vm.load("bin\\count_to_100.mvmb"))
            vm.start();
        return 1;
    }

    bool bRun = true;
    if (strstr(argp[1], "-c") || strstr(argp[1], "-d"))
        bRun = false;
    
    int res = 0;
    std::string in = argp[bRun ? 1 : 2];
    if (!bRun) {
        std::string out = argp[3];

        if (strstr(argp[1], "-c"))
            vm.compile(in, out);
        else
            vm.decompile(in, out);
    } else {
        res = vm.load(in);
        if (res)
            vm.start();
        else {
            cerr << "Could not load MVMB '" << in << "'!\n";
        }
    }
    return res;
}