# Quad32-System-VM
Quad32-System-VM is a lightweight 32 bit VM with a unique instruciton format that contains a 12-bit opcode and 4 5-bit register options.

*PSST, hey you, want to get in the weeds of this VM? Go check out the [Github disscussions](https://github.com/CmdrA43/Quad32-System-VM/discussions), because I don't feel like checking anywhere else.*

Build with:
 * `g++ -Wall -march=native -o3 -ffast-math -flto -fno-semantic-interposition -falign-loops=32 -falign-labels=32 -fopt-info-vec-optimized main.cpp -o quad32_vm` for ARM
 * `g++ -Wall -march=haswell -o3 -mavx2 -mfma -ffast-math -flto -fno-semantic-interposition -falign-loops=32 -falign-labels=32 -fopt-info-vec-optimized main.cpp -o quad32_vm` for x86_64
