#ifndef VM_H 
#define VM_H

#include <stdint.h>

#define STACK_MAX 2086
#define REGISTER_MAX 4

#define REG(x) vm->registers[x]
#define STACK(x) vm->stack[vm->rsp + x]
#define FETCH(x) vm->instructions[vm->rip + x]

typedef int vm_result;

typedef enum Instruction_t {
    INS_HALT,  /* terminate the program. */
    INS_IPUSH, /* push immediates value to the stack. */
    INS_PUSH,  /* push values from register to the stack. */
    INS_POP,   /* pop value from the stack to reister. */
    INS_IMOVE, /* copy value from immediate to register. */
    INS_MOVE,  /* copy value from register to another register. */
    INS_JMP,   /* unconditional jump */
} Instruction;

typedef struct VM_t {
    uint64_t registers[REGISTER_MAX]; /* A, B, C, D */
    uint8_t stack[STACK_MAX];
    const uint8_t* instructions;
    uint64_t rsp;
    uint64_t rip;
} VM;

void vm_execute(VM* vm);
VM* vm_init(const uint8_t* instructions);
void vm_deinit(VM* vm);

#endif /* VM_H */
