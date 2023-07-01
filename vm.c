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
    if (vm->rsp >= STACK_MAX) {
        fprintf(stderr, "ERROR: stackoverflow\n");
        exit(-1);
    }

    for (uint8_t i = 0; i < len; i++)
        STACK(i) = operand[i];

    vm->rsp += len;
}

static void push_stack_register(VM* vm, const uint8_t* registers, uint8_t len) {
    if (vm->rsp >= STACK_MAX) {
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

static void evaluate(VM* vm) {
    const uint8_t ins_len = FETCH(0);
    const uint8_t op_code = FETCH(1);

    switch (op_code) {
    case INS_HALT:
        vm->rip += 1;
        return;

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

    case INS_JMP:
        vm->rip = FETCH(2);
        return;

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

VM* vm_init(const uint8_t* instructions) {
    if (!instructions)
        return NULL;

    VM* vm = malloc(sizeof(VM));
    memset(vm->registers, 0, REGISTER_MAX);

    vm->instructions = instructions;
    vm->rip = 0;
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
