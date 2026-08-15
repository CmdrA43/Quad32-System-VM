# Quad32-System-VM
Quad32-System-VM is a lightweight 32 bit VM with a unique instruction format that contains a 12-bit opcode and 4 5-bit register options.

*PSST, hey you, want to get in the weeds of this VM? Go check out the [Github disscussions](https://github.com/CmdrA43/Quad32-System-VM/discussions), because I don't feel like checking anywhere else.*

Build with standard CMake calls `cmake -S . -B build` and `cmake --build build` after cloning into the repo 

## Architecture
The architecture of this VM is a simple RISC like ISA, with a 12-bit opcode space, and 4 full 5-bit register options, for advanced operations inside a 32-bit environment. Memory is stored in a 65536 long 32-bit word array alongside a dedicated stack, also 32-bit word and 65536 words long.\
Take note that this architecture uses an inverted stack, because despite old design patterns and motherboard layouts, I don't really care and doing it "backwards" vibed better.

## ISA
The instruction bits are broken up like so:\
`xxxxxxxxxxxx xxxxx xxxxx xxxxx xxxxx`\
`   opcode      r0    r1    r2    r3 `\
When referring to instructions detailed below, they will be in a tabular format, where each name either corresponds to instruction name or an appropriate name for what the register is used for in the operation. Any instance of "null" means the register is not used in that operation and can be set to any value, as it does not effect execution.

| Name | r0 | r1 | r2 | r3 | Description |
|------|----|----|----|----|-------------|
|OP_HALT|null|null|null|null|Halts the program.|
|OP_LOAD|dest|ptr|offset|null|Load from `memory[registers[ptr] + registers[offset]]` into `registers[dest]`|
|OP_STORE|src|ptr|offset|null|Store `registers[src]` into `memory[registers[ptr] + registers[offset]]`|
|OP_ADD|src1|src2|dest|null|`registers[dest] = registers[src1] + registers[src2]`|
|OP_SUB|src1|src2|dest|null|`registers[dest] = registers[src1] - registers[src2]`|
|OP_MUL|src1|src2|dest|null|`registers[dest] = registers[src1] * registers[src2]`|
|OP_DIV|src1|src2|dest|null|`registers[dest] = registers[src1] / registers[src2]`|
|OP_MULF|src1|src2|dest|null|Same as OP_MUL but with fixed 16.16 support|
|OP_DIVF|src1|src2|dest|null|Same as OP_DIV but with fixed 16.16 support|
|OP_ADDV|src1loc|src2loc|destloc|null|`memory[destloc...destloc+8] = memory[src1loc...src1loc+8] + memory[src2loc...src2loc+8]` assuming `src1loc + 8`, `src2loc + 8`, and `destloc + 8` are all within memory, if not, out of bounds memory error to halt|
|OP_SUBV|src1loc|src2loc|destloc|null|`memory[destloc...destloc+8] = memory[src1loc...src1loc+8] - memory[src2loc...src2loc+8]` assuming `src1loc + 8`, `src2loc + 8`, and `destloc + 8` are all within memory, if not, out of bounds memory error to halt|
|OP_MULV|src1loc|src2loc|destloc|null|`memory[destloc...destloc+8] = memory[src1loc...src1loc+8] * memory[src2loc...src2loc+8]` assuming `src1loc + 8`, `src2loc + 8`, and `destloc + 8` are all within memory, if not, out of bounds memory error to halt|
|OP_NOT|src1|dest|null|null|Bitwise NOT operation|
|OP_AND|src1|src2|dest|null|Bitwise AND operation|
|OP_OR|src1|src2|dest|null|Bitwise OR operation|
|OP_XOR|src1|src2|dest|null|Bitiwse XOR operation|
|OP_SHL|data|amount|dest|null|`registers[dest] = (registers[data] << registers[amount])`|
|OP_SHR|data|amount|dest|null|`registers[dest] = (registers[data] >> registers[amount])`|
|OP_ASHR|data|amount|dest|null|Arithmetic shift right for sign support|
|OP_JE|src1|src2|jump_true|jump_false|If `registers[src1]` and `registers[src2]` are equal, jump to `registers[jump_true]`, otherwise jump to `registers[jump_false]`|
|OP_JL|src1|src2|jump_true|jump_false|If `registers[src1]` is greater than `registers[src2]`, jump to `registers[jump_true]`, otherwise jump to `registers[jump_false]`|
|OP_JG|src1|src2|jump_true|jump_false|If `registers[src1]` is less than `registers[src2]`, jump to `registers[jump_true]`, otherwise jump to `registers[jump_false]`|
|OP_NOP|null|null|null|null|No operation|
|OP_PUSH|data|null|null|null|Push `registers[data]` to the stack|
|OP_POP|dest|null|null|null|Pop from the stack into `registers[dest]`|
|OP_PUSHR|rlow|rhigh|null|null|Push `registers[rlow...rhigh]` to the stack|
|OP_POPR|rlow|rhigh|null|null|Pop from the stack into `registers[rhigh...rlow]`|
|OP_CALL|dest|null|null|null|Push instruction pointer to the stack and jump to `registers[dest]`|
|OP_RET|null|null|null|null|Pop the top value of the stack directly to the instruction pointer|
|OP_GETPC|dest|null|null|null|Set `registers[dest]` to the current value of the instruction pointer|
|OP_OUT|ptr|length|port|null|Output `memory[registers[ptr]...registers[ptr]+registers[length]]` to the designated port|
|OP_IN|ptr|length|port|null|Poll the designated port `length` times and store at `memory[registers[ptr]...registers[ptr]+registers[length]]`|

You can edit desired port names in the port.h file.\
If you have ideas on changing the instruction list, please visit the [Github disscussions](https://github.com/CmdrA43/Quad32-System-VM/discussions) and check for existing threads to give ideas and feedback.

## Calling Convention & Register Designations
When one function calls another, you would use the OP_CALL instruction to jump to the related function, however, this does *not* handle register preservation, which is instead something you, the programmer, must do. In order to preserve registers, you are given some tools to interact with the stack, most notably OP_PUSH, OP_POP, OP_PUSHR, and OP_POPR.

| Register Designation | Function | Caller/Callee |
|-|-|-|
|REG_ZERO|Hardwired 0|Do not push|
|REG_ONE|Hardwired 1|Do not push|
|REG_A0|Function argument 0|Pushed by caller, setup by caller|
|REG_A1|Function argument 1|Pushed by caller, setup by caller|
|REG_A2|Function argument 2|Pushed by caller, setup by caller|
|REG_A3|Function argument 3|Pushed by caller, setup by caller|
|REG_V0|Return Value 0|Pushed by caller, setup by callee|
|REG_V1|Return Value 1|Pushed by caller, setup by callee|
|REG_T0|Temporary Register 0|Pushed by caller|
|REG_T1|Temporary Register 1|Pushed by caller|
|REG_T2|Temporary Register 2|Pushed by caller|
|REG_T3|Temporary Register 3|Pushed by caller|
|REG_T4|Temporary Register 4|Pushed by caller|
|REG_T5|Temporary Register 5|Pushed by caller|
|REG_T6|Temporary Register 6|Pushed by caller|
|REG_T7|Temporary Register 7|Pushed by caller|
|REG_S0|Saved Register 0|Pushed by callee|
|REG_S1|Saved Register 1|Pushed by callee|
|REG_S2|Saved Register 2|Pushed by callee|
|REG_S3|Saved Register 3|Pushed by callee|
|REG_S4|Saved Register 4|Pushed by callee|
|REG_S5|Saved Register 5|Pushed by callee|
|REG_S6|Saved Register 6|Pushed by callee|
|REG_S7|Saved Register 7|Pushed by callee|
|REG_TA0|Target Register 0|Pushed by caller|
|REG_TA1|Target Register 1|Pushed by caller|
|REG_TA2|Target Register 2|Pushed by caller|
|REG_TA3|Target Register 3|Pushed by caller|
|REG_RES0|RESERVED|Do not push|
|REG_RES1|RESERVED|Do not push|
|REG_GP|Global Pointer|Do not push|
|REG_TMP|Temp System Register|Do not push|

When calling a function, the callee should be able to expect that:
 * REG_A0-3 contain relevant arguments
 * REG_V0-1 are safe to be overwritten
 * REG_T0-7 are safe to be overwritten
 * REG_S0-7 should be pushed to the stack as needed and then popped back off before returning
 * and REG_TA0-3 should be safe to be overwritten

Please note that REG_RES0-1 are obviously reserved for future functionality, REG_GP is for a global pointer to say a database, or some other important location, and REG_TMP is reserved for future system functionality for things like operating systems and compilers.\
If you have ideas on changing the calling convention, please visit the [Github disscussions](https://github.com/CmdrA43/Quad32-System-VM/discussions) and check for existing threads to give ideas and feedback.
