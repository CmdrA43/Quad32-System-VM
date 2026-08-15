#include "include/cpu/cpu.h"
#include "include/gpu/gpu.h"

int main() {
    quad32VM vm;

    uint32_t bytecode[ARRAY_SIZE];

    std::cout << "Booting Virtual Silicon..." << std::endl;
    vm.Run(bytecode);
    std::cout << "Execution Halted Successfully." << std::endl;

    return 0;
}
