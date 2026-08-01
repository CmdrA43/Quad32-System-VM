#include <vector>
#include <cstdint>
#include <iostream>
#include <iomanip>

const uint32_t ARRAY_SIZE = 10;

#define MAKE_INS(op, r0, r1, r2, r3) \
    (((uint32_t)(op)  & 0xFF) << 20) | \
    (((uint32_t)(r0)  & 0x1F) << 15) | \
    (((uint32_t)(r1)  & 0x1F) << 10) | \
    (((uint32_t)(r2)  & 0x1F) << 5)  | \
    ((uint32_t)(r3)  & 0x1F)

enum Opcode : uint16_t {
	OP_HALT = 0x000,
	OP_LOAD = 0x001, // reg dest, reg ptr, reg offset (from ram to register)
	OP_STORE = 0x002, // reg src, reg ptr, reg offset (from register to ram)
	OP_ADD = 0x003, // reg src1, reg src2, reg dest
	OP_SUB = 0x004, // reg src1, reg src2, reg dest
	OP_NOT = 0x005, // reg src1, reg dest
	OP_AND = 0x006, // reg src1, reg src2, reg dest
	OP_OR = 0x007, // reg src1, reg src2, reg dest
	OP_XOR = 0x008, // reg src1, reg src2, reg dest
	OP_SHL = 0x009, // reg src1, reg src2, reg dest
	OP_SHR = 0x00A, // reg src1, reg src2, reg dest
	OP_JE = 0x00B, // reg src1, reg src2, reg jump_true, reg jump_false
	OP_JL = 0x00C, // reg src1, reg src2, reg jump_true, reg jump_false
	OP_JG = 0x00D, // reg src1, reg src2, reg jump_true, reg jump_false
	OP_NOP = 0x00E, // nop
	OP_PUSH = 0x00F, // reg data
	OP_POP = 0x010, // reg dest
	OP_PUSHR = 0x011, // reg rlow, reg rhigh
	OP_POPR = 0x012, // reg rlow, reg rhigh
	OP_CALL = 0x013, // reg dest
	OP_RET = 0x014,
	OP_GETPC = 0x015, // reg dest
	OP_OUT = 0x016, // reg ptr, reg length, reg port
	OP_IN = 0x017 // reg ptr, reg length, reg port
};

enum regCallConv : uint8_t{
	REG_ZERO = 0x00,
	REG_ONE = 0x01,
	REG_A0 = 0x02,
	REG_A1 = 0x03,
	REG_A2 = 0x04,
	REG_A3 = 0x05,
	REG_V0 = 0x06,
	REG_V1 = 0x07,
	REG_T0 = 0x08,
	REG_T1 = 0x09,
	REG_T2 = 0x0A,
	REG_T3 = 0x0B,
	REG_T4 = 0x0C,
	REG_T5 = 0x0D,
	REG_T6 = 0x0E,
	REG_T7 = 0x0F,
	REG_S0 = 0x10,
	REG_S1 = 0x11,
	REG_S2 = 0x12,
	REG_S3 = 0x13,
	REG_S4 = 0x14,
	REG_S5 = 0x15,
	REG_S6 = 0x16,
	REG_S7 = 0x17,
	REG_TA0 = 0x18,
	REG_TA1 = 0x19,
	REG_TA2 = 0x1A,
	REG_TA3 = 0x1B,
	REG_RES0 = 0x1C,
	REG_RES1 = 0x1D,
	REG_GP = 0x1E,
	REG_TMP = 0x1F
};

struct quad32VM{
	public:
	uint32_t memory[65536] = {0};
	uint32_t stack[65536] = {0};
	uint32_t registers[32] = {0};
	uint32_t instruction_ptr = 0;
	uint32_t stack_ptr = 0;
	
	void regDump(uint8_t opcode) {
        std::cout << "----- CYCLE REG DUMP -----\n";
        std::cout << "IP: " << instruction_ptr << "\n";
        std::cout << "OP: 0x" << std::hex << (int)opcode << std::dec << "\n";
        for(int i = 0; i < 32; i++) {
            std::cout << "R" << std::setw(2) << i << ": " << registers[i] << "\t";
            if ((i + 1) % 4 == 0) std::cout << "\n";
        }
        std::cout << "STACK (TOP DOWN):\n";
		if (stack_ptr == 0) {
			std::cout << "  [Empty Stack]\n";
		} else {
			int limit = (stack_ptr > 8) ? (stack_ptr - 8) : 0;
			for(int i = static_cast<int>(stack_ptr) - 1; i >= limit; i--) {
				std::cout << "  Slot [" << std::setw(5) << i << "] : " << stack[i] << "\n";
			}
		}
        std::cout << "--------------------------\n";
    }
	/*
	 * R0 is dedicated zero value
	 * R1 is dedicated one value
	 * */
	void Run(uint32_t (&bytecode)[ARRAY_SIZE]){
		registers[0] = 0;
		registers[1] = 1;
		while (instruction_ptr < ARRAY_SIZE - 1){
			uint32_t instruction = bytecode[instruction_ptr++];
			uint16_t opcode = (instruction >> 20) & 0xFFF;
			uint8_t r0 = (instruction >> 15) & 0x1F;
			uint8_t r1 = (instruction >> 10) & 0x1F;
			uint8_t r2 = (instruction >> 5) & 0x1F;
			uint8_t r3 = instruction & 0x1F;
			switch(opcode){
				case OP_HALT:
					regDump(opcode);
					return;
				case OP_LOAD:{
					uint8_t dest_reg = r0;
					uint8_t ram_src = r1;
					registers[dest_reg] = memory[registers[ram_src]];
					break;
				}
				case OP_STORE:{
					uint8_t reg_data = r0;
					uint8_t ram_dest = r1;
					memory[registers[ram_dest]] = registers[reg_data];
					break;
				}
				case OP_ADD:{
					uint8_t reg_src1 = r0;
					uint8_t reg_src2 = r1;
					uint8_t reg_dest = r2;
					registers[reg_dest] = (registers[reg_src1] + registers[reg_src2]);
					break;
				}
				case OP_SUB:{
					uint8_t reg_src1 = r0;
					uint8_t reg_src2 = r1;
					uint8_t reg_dest = r2;
					registers[reg_dest] = (registers[reg_src1] - registers[reg_src2]);
					break;
				}
				case OP_NOT:{
					uint8_t reg_src1 = r0;
					uint8_t reg_dest = r1;
					registers[reg_dest] = ~registers[reg_src1];
					break;
				}
				case OP_AND:{
					uint8_t reg_src1 = r0;
					uint8_t reg_src2 = r1;
					uint8_t reg_dest = r2;
					registers[reg_dest] = (registers[reg_src1] & registers[reg_src2]);
					break;
				}
				case OP_OR:{
					uint8_t reg_src1 = r0;
					uint8_t reg_src2 = r1;
					uint8_t reg_dest = r2;
					registers[reg_dest] = (registers[reg_src1] | registers[reg_src2]);
					break;
				}
				case OP_XOR:{
					uint8_t reg_src1 = r0;
					uint8_t reg_src2 = r1;
					uint8_t reg_dest = r2;
					registers[reg_dest] = (registers[reg_src1] ^ registers[reg_src2]);
					break;
				}
				case OP_SHL:{
					uint8_t reg_src1 = r0;
					uint8_t reg_src2 = r1;
					uint8_t reg_dest = r2;
					registers[reg_dest] = (registers[reg_src1] << registers[reg_src2]);
					break;
				}
				case OP_SHR:{
					uint8_t reg_src1 = r0;
					uint8_t reg_src2 = r1;
					uint8_t reg_dest = r2;
					registers[reg_dest] = (registers[reg_src1] >> registers[reg_src2]);
					break;
				}
				case OP_JE:{
					uint8_t reg_src1 = r0;
					uint8_t reg_src2 = r1;
					uint8_t reg_true = r2;
					uint8_t reg_false = r3;
					if(static_cast<int32_t>(registers[reg_src1]) == static_cast<int32_t>(registers[reg_src2])){
						instruction_ptr = registers[reg_true];
					} else {
						instruction_ptr = registers[reg_false];
					}
					break;
				}
				case OP_JL:{
					uint8_t reg_src1 = r0;
					uint8_t reg_src2 = r1;
					uint8_t reg_true = r2;
					uint8_t reg_false = r3;
					if(static_cast<int32_t>(registers[reg_src1]) < static_cast<int32_t>(registers[reg_src2])){
						instruction_ptr = registers[reg_true];
					} else {
						instruction_ptr = registers[reg_false];
					}
					break;
				}
				case OP_JG:{
					uint8_t reg_src1 = r0;
					uint8_t reg_src2 = r1;
					uint8_t reg_true = r2;
					uint8_t reg_false = r3;
					if(static_cast<int32_t>(registers[reg_src1]) > static_cast<int32_t>(registers[reg_src2])){
						instruction_ptr = registers[reg_true];
					} else {
						instruction_ptr = registers[reg_false];
					}
					break;
				}
				case OP_NOP:{
					break;
				}
				case OP_PUSH:{
					uint8_t reg_data = r0;
					if(stack_ptr >= 65536){
						std::cerr << "STACK OVERFLOW: KILLING VM PROCESS\n";
						regDump(opcode);
						return;
					}
					stack[stack_ptr] = registers[reg_data];
					stack_ptr++;
					break;
				}
				case OP_POP:{
					uint8_t reg_dest = r0;
					if(stack_ptr <= 0){
						std::cerr << "STACK UNDERFLOW: KILLING VM PROCESS\n";
						regDump(opcode);
						return;
					}
					stack_ptr--;
					registers[reg_dest] = stack[stack_ptr];
					break;
				}
				case OP_PUSHR:{
					uint8_t reg_low = r0;
					uint8_t reg_high = r1;
					for(uint8_t i = reg_low; i <= reg_high; i++){
						if(stack_ptr >= 65536){
							std::cerr << "STACK OVERFLOW: KILLING VM PROCESS\n";
							regDump(opcode);
							return;
						}
						stack[stack_ptr] = registers[i];
						stack_ptr++;
					}
					break;
				}
				case OP_POPR:{
					uint8_t reg_low = r0;
					uint8_t reg_high = r1;
					for(int32_t i = reg_high; i >= reg_low; i--){
						if(stack_ptr <= 0){
							std::cerr << "STACK UNDERFLOW: KILLING VM PROCESS\n";
							regDump(opcode);
							return;
						}
						stack_ptr--;
						registers[i] = stack[stack_ptr];
					}
					break;
				}
				case OP_CALL:{
					uint8_t reg_addr = r0;
					if(stack_ptr >= 65536){
						std::cerr << "STACK OVERFLOW: KILLING VM PROCESS\n";
						regDump(opcode);
						return;
					}
					stack[stack_ptr] = instruction_ptr;
					stack_ptr++;
					instruction_ptr = registers[reg_addr];
					break;
				}
				case OP_RET:{
					if(stack_ptr <= 0){
						std::cerr << "STACK UNDERFLOW: KILLING VM PROCESS\n";
						regDump(opcode);
						return;
					}
					stack_ptr--;
					instruction_ptr = stack[stack_ptr];
					break;
				}
				case OP_GETPC:{
					uint8_t reg_dest = r0;
					registers[reg_dest] = instruction_ptr;
					break;
				}
				case OP_OUT:{
					uint8_t reg_data = r0;
					uint8_t port = r1;
					switch(registers[port]){
						case 0:
							std::cout << (char)registers[reg_data];
							break;
						case 1:
							std::cout << (int)registers[reg_data];
							break;
						default:
							break;
							
					}
					break;
				}
				case OP_IN:{
					uint8_t reg_dest = r0;
					uint8_t port = r1;
					switch(registers[port]){
						case 0:
							std::cin >> registers[reg_dest];
							break;
						case 1:
							std::cin >> registers[reg_dest];
							break;
						default:
							break;
					}
					break;
				}
				default:
					break;
			}
			registers[0] = 0;
			registers[1] = 1;
			//regDump(opcode);
		}
	}
};
int main() {
    quad32VM vm;

    uint32_t bytecode[ARRAY_SIZE];


    std::cout << "Booting Virtual Silicon..." << std::endl;
    vm.Run(bytecode);
    std::cout << "Execution Halted Successfully." << std::endl;

    return 0;
}
