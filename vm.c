#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

static void push_stack_byte(VM* vm, uint8_t byte) {
    if (vm->rsp >= STACK_MAX) {
        fprintf(stderr, "ERROR: stackoverflow\n");
        exit(-1);
    }

    STACK(0) = byte;
    vm->rsp += 1;
}

static void pop_stack_qword(VM* vm, uint8_t dst) {
    uint8_t* bytes = (uint8_t*)&REG(dst);
    uint8_t* stack_bytes = &STACK(-8);

    uint64_t current_rsp = vm->rsp;
    uint64_t diff = current_rsp - sizeof(uint64_t);

    if (vm->rsp == 0 || diff > current_rsp) {
        fprintf(stderr, "ERROR: stackoverflow\n");
        exit(-1);
    }

    for (uint8_t i = 0; i < sizeof(uint64_t); i++)
        bytes[i] = stack_bytes[i];

    vm->rsp -= 8;
}

static void push_stack_immediate(VM* vm, const uint8_t* operand, uint8_t len) {
    if (vm->rsp + len >= STACK_MAX) {
        fprintf(stderr, "ERROR: stackoverflow\n");
        exit(-1);
    }

    for (uint8_t i = 0; i < len; i++)
        STACK(i) = operand[i];

    while (len < sizeof(uint64_t)) {
        STACK(len) = 0x00;
        len += 1;
    }

    vm->rsp += len;
}

static void push_stack_register(VM* vm, const uint8_t* registers, uint8_t len) {
    if (vm->rsp + len >= STACK_MAX) {
        fprintf(stderr, "ERROR: stackoverflow\n");
        exit(-1);
    }

    for (uint8_t i = 0; i < len; i++) {
        uint8_t* bytes = (uint8_t*)&REG(registers[i]);

        for (uint8_t j = 0; j < sizeof(uint64_t); j++)
            push_stack_byte(vm, bytes[j]);
    }
}

static void move_immediate(VM* vm, uint8_t dst, const uint8_t* operand, uint8_t len) {
    uint8_t* bytes = (uint8_t*)&REG(dst);

    for (uint8_t i = 0; i < len; i++)
        bytes[i] = operand[i];
}

static void set_flags(VM* vm, uint64_t lhs, uint64_t rhs) {
    FEQ   = lhs == rhs;
    FNEQ  = lhs != rhs;
    FGT   = lhs >  rhs;
    FLT   = lhs <  rhs;
    FGTEQ = lhs >= rhs;
    FLTEQ = lhs <= rhs;
}

/* compares register with immediate. */
static void compare_immediate(VM* vm, uint8_t lhs, const uint8_t* operand, uint8_t len) {
    uint64_t rhs = 0;
    uint8_t* bytes = (uint8_t*)&rhs;

    for (uint8_t i = 0; i < len; i++)
        bytes[i] = operand[i];

    set_flags(vm, REG(lhs), rhs);
}

/* compares register with another register. */
static void compare_register(VM* vm, uint8_t lhs, uint8_t rhs) {
    set_flags(vm, REG(lhs), REG(rhs));
}

static void arithmetic_op_immediate(VM* vm, uint8_t dst, const uint8_t* operand, uint8_t len, uint8_t op) {
    uint64_t rhs = 0;
    uint8_t* bytes = (uint8_t*)&rhs;

    for (uint8_t i = 0; i < len; i++)
        bytes[i] = operand[i];

    switch (op) {
    case '+':
        REG(dst) += rhs;
        break;
    case '-':
        REG(dst) -= rhs;
        break;
    case '*':
        REG(dst) *= rhs;
        break;
    case '/':
        REG(dst) /= rhs;
        break;
    default:
        break;
    }
}

static void arithmetic_op_register(VM* vm, uint8_t dst, const uint8_t* registers, uint8_t len, uint8_t op) {
    switch (op) {
    case '+':
        for (uint8_t i = 0; i < len; i++)
            REG(dst) += REG(registers[i]);

        break;
    case '-':
        for (uint8_t i = 0; i < len; i++)
            REG(dst) -= REG(registers[i]);

        break;
    case '*':
        for (uint8_t i = 0; i < len; i++)
            REG(dst) *= REG(registers[i]);

        break;
    case '/':
        for (uint8_t i = 0; i < len; i++)
            REG(dst) /= REG(registers[i]);

        break;
    default:
        break;
    }
}

static void load_rip_immediate(VM* vm, const uint8_t* operand, uint8_t len) {
    uint64_t new_rip = 0;
    uint8_t* bytes = (uint8_t*)&new_rip;

    for (uint8_t i = 0; i < len; i++)
        bytes[i] = operand[i];

    vm->rip = new_rip;
}

static void evaluate(VM* vm) {
    const uint8_t ins_len = FETCH(0);
    const uint8_t op_code = FETCH(1);

    switch (op_code) {
    case INS_HALT:
        vm->rip += 1;
        return;

    case INS_IADD:
        arithmetic_op_immediate(vm, FETCH(2), &FETCH(3), ins_len - 3, '+');
        break;

    case INS_ISUB:
        arithmetic_op_immediate(vm, FETCH(2), &FETCH(3), ins_len - 3, '-');
        break;

    case INS_IMUL:
        arithmetic_op_immediate(vm, FETCH(2), &FETCH(3), ins_len - 3, '*');
        break;

    case INS_IDIV:
        arithmetic_op_immediate(vm, FETCH(2), &FETCH(3), ins_len - 3, '/');
        break;

    case INS_ADD:
        arithmetic_op_register(vm, FETCH(2), &FETCH(3), ins_len - 3, '+');
        break;

    case INS_SUB:
        arithmetic_op_register(vm, FETCH(2), &FETCH(3), ins_len - 3, '-');
        break;

    case INS_MUL:
        arithmetic_op_register(vm, FETCH(2), &FETCH(3), ins_len - 3, '*');
        break;

    case INS_DIV:
        arithmetic_op_register(vm, FETCH(2), &FETCH(3), ins_len - 3, '/');
        break;

    case INS_IPUSH:
        push_stack_immediate(vm, &FETCH(2), ins_len - 2);
        break;

    case INS_PUSH:
        push_stack_register(vm, &FETCH(2), ins_len - 2);
        break;

    case INS_POP:
        pop_stack_qword(vm, FETCH(2));
        break;

    case INS_IMOVE:
        move_immediate(vm, FETCH(2), &FETCH(3), ins_len - 3);
        break;

    case INS_MOVE:
        REG(FETCH(2)) = REG(FETCH(3));
        break;

    case INS_ICMP:
        compare_immediate(vm, FETCH(2), &FETCH(3), ins_len - 3);
        break;

    case INS_CMP:
        compare_register(vm, FETCH(2), FETCH(3));
        break;

    case INS_JMP:
        load_rip_immediate(vm, &FETCH(2), ins_len - 2);
        return;

    case INS_JE:
        if (FEQ) {
            load_rip_immediate(vm, &FETCH(2), ins_len - 2);
            return;
        }

        break;

    case INS_JNE:
        if (FNEQ) {
            load_rip_immediate(vm, &FETCH(2), ins_len - 2);
            return;
        }

        break;

    case INS_JG:
        if (FGT) {
            load_rip_immediate(vm, &FETCH(2), ins_len - 2);
            return;
        }

        break;

    case INS_JL:
        if (FLT) {
            load_rip_immediate(vm, &FETCH(2), ins_len - 2);
            return;
        }

        break;

    case INS_JGE:
        if (FGTEQ) {
            load_rip_immediate(vm, &FETCH(2), ins_len - 2);
            return;
        }

        break;

    case INS_JLE:
        if (FLTEQ) {
            load_rip_immediate(vm, &FETCH(2), ins_len - 2);
            return;
        }

        break;
    
    default:
        fprintf(stderr, "ERROR: unknown opcode: '%2x'\n", op_code);
        exit(-1);
    }

    vm->rip += ins_len;
}

void vm_execute(VM* vm) {
    while (FETCH(0) != INS_HALT)
        evaluate(vm);
}

VM* vm_init(const uint8_t* instructions, uint64_t start_rip) {
    if (!instructions)
        return NULL;

    VM* vm = malloc(sizeof(VM));
    memset(vm->flags, 0, FLAGS_MAX);
    memset(vm->registers, 0, REGISTER_MAX * sizeof(uint64_t));

    vm->instructions = instructions;
    vm->rip = start_rip;
    vm->rsp = 0;

    return vm;
}

void vm_deinit(VM* vm) {
    if (!vm)
        return;

    vm->instructions = NULL;
    vm->rip = 0;
    vm->rsp = 0;

    free(vm);
}
