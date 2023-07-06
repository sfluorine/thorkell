#ifndef PARSER_H
#define PARSER_H

#include "vm.h"
#include "cvector.h"

typedef struct ParsedInstruction_t {
    Instruction instruction;
    cvector_vector_type(uint8_t) operands;
    uint8_t size;
} ParsedInstruction;

ParsedInstruction parsed_instruction_init(Instruction instruction, cvector_vector_type(uint8_t) operands, uint8_t size);
void parsed_instruction_deinit(ParsedInstruction parsed_instruction);

int parser_init(const char* input);
cvector_vector_type(ParsedInstruction) parser_start();

#endif /* PARSER_H */
