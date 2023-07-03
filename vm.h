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
    INS_HALT,  /* terminate the program. */
    INS_IADD,  
    INS_ISUB,
    INS_IMUL,
    INS_IDIV,
    INS_ADD,  
    INS_SUB,
    INS_MUL,
    INS_DIV,
    INS_IPUSH, /* push immediate values to the stack. */
    INS_PUSH,  /* push values from register to the stack. */
    INS_POP,   /* pop value from the stack to reister. */
    INS_IMOVE, /* copy value from immediate to register. */
    INS_MOVE,  /* copy value from register to another register. */
    INS_ICMP,  /* compare register with an immediate value. */
    INS_CMP,   /* compare register with another register. */
    INS_JMP,   /* unconditional jump. */
    INS_JE,    /* jump if equal. */
    INS_JNE,    /* jump if not equal. */
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
VM* vm_init(const uint8_t* instructions);
void vm_deinit(VM* vm);

#endif /* VM_H */
