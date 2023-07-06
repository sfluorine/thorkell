#ifndef VM_H 
#define VM_H

#include <stdint.h>

#define STACK_MAX 2086
#define FLAGS_MAX 6
#define REGISTER_MAX 4

#define REG(x)   vm->registers[x]
#define STACK(x) vm->stack[vm->rsp + x]
#define FETCH(x) vm->instructions[vm->rip + x]

#define FEQ   vm->flags[0]
#define FNEQ  vm->flags[1]
#define FGT   vm->flags[2]
#define FLT   vm->flags[3]
#define FGTEQ vm->flags[4]
#define FLTEQ vm->flags[5]

typedef enum Instruction_t {
    INS_HALT,
    INS_IADD,
    INS_ISUB,
    INS_IMUL,
    INS_IDIV,
    INS_ADD,
    INS_SUB,
    INS_MUL,
    INS_DIV,
    INS_IPUSH,
    INS_PUSH,
    INS_POP,
    INS_IMOVE,
    INS_MOVE,
    INS_ICMP,
    INS_CMP,
    INS_JMP,
    INS_JE,
    INS_JNE,
    INS_JG,
    INS_JL,
    INS_JGE,
    INS_JLE,
} Instruction;

typedef struct VM_t {
    uint64_t registers[REGISTER_MAX]; /* A, B, C, D */
    uint8_t flags[FLAGS_MAX];
    uint8_t stack[STACK_MAX];
    const uint8_t* instructions;
    uint64_t rsp;
    uint64_t rip;
} VM;

void vm_execute(VM* vm);
VM* vm_init(const uint8_t* instructions, uint64_t start_rip);
void vm_deinit(VM* vm);

#endif /* VM_H */
