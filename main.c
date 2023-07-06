#include <stdio.h>

#include "cvector.h"
#include "parser.h"
#include "vm.h"

static cvector_vector_type(uint8_t) parsed_instructions_codegen(cvector_vector_type(ParsedInstruction) parsed_instructions) {
    cvector_vector_type(uint8_t) instructions = NULL;

    for (uint64_t i = 0; i < cvector_size(parsed_instructions); i++) {
        cvector_push_back(instructions, parsed_instructions[i].size);
        cvector_push_back(instructions, parsed_instructions[i].instruction);

        for (uint64_t j = 0; j < cvector_size(parsed_instructions[i].operands); j++)
            cvector_push_back(instructions, parsed_instructions[i].operands[j]);
    }

    return instructions;
}

int main(void) {
    parser_init("add RA, 34 add RB, 35 add RC, RA, RB, RA, RB, RA, RB, RA, RB, RA, RB, RC halt");

    cvector_vector_type(ParsedInstruction) parsed_instructions = parser_start();
    cvector_vector_type(uint8_t) instructions = parsed_instructions_codegen(parsed_instructions);

    VM* vm = vm_init(instructions);
    vm_execute(vm);

    for (uint8_t i = 0; i < REGISTER_MAX; i++)
        printf("%lu\n", vm->registers[i]);

    vm_deinit(vm);

    for (uint64_t i = 0; i < cvector_size(parsed_instructions); i++)
        parsed_instruction_deinit(parsed_instructions[i]);

    cvector_free(instructions);
    cvector_free(parsed_instructions);
}
