#ifndef PARSER_H
#define PARSER_H

#include "cvector.h"
#include "vm.h"
#include "lexer.h"

typedef struct ParsedInstruction_t {
    Instruction instruction;
    cvector_vector_type(uint8_t) operands;
    uint8_t size;
} ParsedInstruction;

ParsedInstruction parsed_instruction_init(Instruction instruction, cvector_vector_type(uint8_t) operands, uint8_t size);
void parsed_instruction_deinit(ParsedInstruction parsed_instruction);

int parser_init(const char* input);
void parser_deinit();
cvector_vector_type(ParsedInstruction) parser_start(uint64_t* start_rip);

#endif /* PARSER_H */
