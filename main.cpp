#include "include/cpu/cpu.h"
#include "include/gpu/gpu.h"
#include <atomic>
#include <thread>

std::atomic<uint32_t> GPULane;
std::atomic<bool> dataReady;

void gpuThread(){
	GPU gpu(16, 16, 512, 256, vector<int>{512, 512});
	
	// initialize your GPU memory with whatever here
	
	gpu.commandQueue.tickFromHardware = std::function<bool(uint32_t&)>([](uint32_t& value){
		if (!dataReady) { return false; }
		value = GPULane;
		dataReady = false;
		return true;
	});
	
	gpu.Run();
};

int main() {
    quad32VM vm;

    uint32_t bytecode[ARRAY_SIZE];
    
    vm.ports[GPUPort].tickToHardware = std::function<bool(uint32_t)>([](uint32_t value){
		if(dataReady) { return false; }
		GPULane = value;
		dataReady = true;
		return true;
	});

    std::cout << "Booting Virtual Silicon..." << std::endl;
    vm.Run(bytecode);
    std::cout << "Execution Halted Successfully." << std::endl;

    return 0;
}
