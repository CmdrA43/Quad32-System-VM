#include "include/cpu/cpu.h"
#include "include/gpu/gpu.h"
#include <atomic>
#include <thread>
#include <cstring>
#include <SDL3/SDL.h>

std::atomic<uint32_t> GPULane;
std::atomic<bool> gpuDataReady(false);

void gpuThread(GPU& gpu){
	// initialize your GPU memory with whatever here
	
	gpu.commandQueue.tickFromHardware = std::function<bool(uint32_t&)>([](uint32_t& value){
		if (!gpuDataReady) { return false; }
		value = GPULane;
		gpuDataReady = false;
		return true;
	});
	
	gpu.Run();
};

std::atomic<uint32_t> CPUInLane[3]; // event type, data 1, data 2
std::atomic<bool> CPUDataReady(false);
static uint32_t currentReadIndex = 0;

void cpuThread(quad32VM& vm, uint32_t bytecode[ARRAY_SIZE]){
	// initialize your CPU memory with whatever here
	
	vm.ports[GPUPort].tickToHardware = std::function<bool(uint32_t)>([](uint32_t value){
		if(gpuDataReady) { return false; }
		GPULane = value;
		gpuDataReady = true;
		return true;
	});
	
	vm.ports[SDL].tickFromHardware = std::function<bool(uint32_t&)>([](uint32_t& value){
		if(!CPUDataReady) { return false; }
		value = CPUInLane[currentReadIndex];
		currentReadIndex++;
		if(currentReadIndex == 3){
			CPUDataReady = false;
			currentReadIndex = 0;
		}
		return true;
	});
	
	vm.Run(bytecode);
};

int main() {
	bool systemRunning = true;
	
	GPU gpu(16, 16, 512, 256, vector<int>{512, 512});
	quad32VM vm;

    uint32_t bytecode[ARRAY_SIZE];
    
    // define the bytecode here
	
	std::thread GPUThread = std::thread(gpuThread, std::ref(gpu));
	std::thread CPUThread = std::thread(cpuThread, std::ref(vm), bytecode);
	
    std::cout << "Booting Virtual Silicon..." << std::endl;
    
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_Window* window = SDL_CreateWindow("VM", screenSize.x, screenSize.y, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    SDL_Texture* screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, screenSize.x, screenSize.y);
    
    while (systemRunning){
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			if (event.type == SDL_EVENT_QUIT) {
				systemRunning = false;
			} else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
				if(!CPUDataReady){
					if (event.key.repeat) { continue; }
					CPUInLane[0] = 0x00000001;
					CPUInLane[2] = static_cast<uint32_t>(event.key.scancode);
					CPUInLane[1] = (event.type == SDL_EVENT_KEY_DOWN) ? 1 : 0;
				}
				
			}
			
		}
		
		void* pixels; int pitch;
		SDL_LockTexture(screenTexture, nullptr, &pixels, &pitch);
		std::memcpy(pixels, gpu.screen.data(), screenSize.x * screenSize.y * 4);
		SDL_UnlockTexture(screenTexture);
		
		SDL_RenderClear(renderer);
		SDL_RenderTexture(renderer, screenTexture, nullptr, nullptr);
		SDL_RenderPresent(renderer);
		SDL_Delay(8);
	};
    
    SDL_DestroyTexture(screenTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	if (CPUThread.joinable()) CPUThread.join();
	if (GPUThread.joinable()) GPUThread.join();

    return 0;
}
