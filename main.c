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
    parser_init("; this program is computing the factorial of 10\nfactorial: move RA, 1 move RB, 10 loop: mul RA, RB sub RB, 1 cmp RB, 0 jg loop halt start: jmp factorial");

    uint64_t start_rip = 0;
    cvector_vector_type(ParsedInstruction) parsed_instructions = parser_start(&start_rip);

    parser_deinit();

    cvector_vector_type(uint8_t) instructions = parsed_instructions_codegen(parsed_instructions);

    VM* vm = vm_init(instructions, start_rip);
    vm_execute(vm);

    for (uint8_t i = 0; i < REGISTER_MAX; i++)
        printf("%lu\n", vm->registers[i]);

    vm_deinit(vm);

    for (uint64_t i = 0; i < cvector_size(parsed_instructions); i++)
        parsed_instruction_deinit(parsed_instructions[i]);

    cvector_free(instructions);
    cvector_free(parsed_instructions);
}
