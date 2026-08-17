#ifndef INPUTHELPER_H
#define INPUTHELPER_H

enum portMap : uint32_t {
	GPUPort = 0x00000000,
	KEYBOARD = 0x00000001
};

void initCPUIn(quad32VM& vm){
	vm.ports[KEYBOARD]
};

#endif
