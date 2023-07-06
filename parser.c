#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "lexer.h"

typedef struct Symbol_t {
    Span span;
    uint64_t ip;
} Symbol;

static Symbol symbol_init(Span span, uint64_t ip) {
    return (Symbol) {
        .span = span,
        .ip = ip,
    };
}

static Token g_current;

static cvector_vector_type(Symbol) g_symtab = NULL;

static int symtab_lookup(Span span) {
    for (int i = 0; i < cvector_size(g_symtab); i++) {
        if (span_equals(span, g_symtab[i].span))
            return i;
    }

    return -1;
}

int parser_init(const char* input) {
    if (!lexer_init(input))
        return 0;

    g_current = lexer_get_token();
    return 1;
}

void parser_deinit() {
    if (g_symtab != NULL)
        cvector_free(g_symtab);
}

static int expect(TokenKind kind) {
    return g_current.kind == kind;
}

static int is_eof() {
    return expect(TOK_EOF);
}

static uint8_t token_to_register(Token token) {
    switch (token.kind) {
    case TOK_REG_A:
        return 0x00;
    case TOK_REG_B:
        return 0x01;
    case TOK_REG_C:
        return 0x02;
    case TOK_REG_D:
        return 0x03;
    default:
        fprintf(stderr, "ERROR: cannot convert non register to byte!\n");
        exit(1);
    }
}

static void advance() {
    if (!expect(TOK_EOF))
        g_current = lexer_get_token();
}

static void match(TokenKind kind) {
    if (!expect(kind)) {
        fprintf(stderr, "(%zu:%zu) ERROR: unexpected token: ", g_current.line, g_current.col);
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
    }

    advance();
}

static void match_register() {
    switch (g_current.kind) {
    case TOK_REG_A:
    case TOK_REG_B:
    case TOK_REG_C:
    case TOK_REG_D:
        advance();
        break;
    default:
        fprintf(stderr, "(%zu:%zu) ERROR: expected register but got: ", g_current.line, g_current.col);
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
    }
}

cvector_vector_type(ParsedInstruction) parser_start(uint64_t* start_rip) {
    cvector_vector_type(ParsedInstruction) parsed_instructions = NULL;

    uint64_t rip = 0;

    while (!is_eof()) {
        if (expect(TOK_LABLE)) {
            if (span_equals(span_from("start"), g_current.span))
                *start_rip = rip;

            cvector_push_back(g_symtab, symbol_init(g_current.span, rip));
            advance();

            continue;
        }

        if (expect(TOK_HALT)) {
            cvector_push_back(parsed_instructions, parsed_instruction_init(INS_HALT, NULL, 2));
            advance();

            rip += 2;
            continue;
        }

        if (expect(TOK_ADD)) {
            uint8_t size = 3;

            advance();

            Token dst_reg = g_current;
            match_register();

            match(TOK_COMMA);

            cvector_vector_type(uint8_t) operands = NULL;
            cvector_push_back(operands, token_to_register(dst_reg));

            if (expect(TOK_IMMEDIATE)) {
                size += sizeof(uint64_t);

                Token immediate = g_current;
                advance();

                uint64_t immediate_value = strtoul(immediate.span.data, NULL, 10);
                uint8_t* bytes = (uint8_t*)&immediate_value;

                for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                    cvector_push_back(operands, bytes[i]);

                ParsedInstruction iop = parsed_instruction_init(INS_IADD, operands, size);
                cvector_push_back(parsed_instructions, iop);

                rip += size;
                continue;
            }

            uint8_t iteration = 0;

            do {
                if (iteration != 0 && expect(TOK_COMMA))
                    advance();

                Token src_reg = g_current;
                match_register();

                cvector_push_back(operands, token_to_register(src_reg));
                
                iteration += 1;
                size += 1;
            } while (!is_eof() && expect(TOK_COMMA));

            ParsedInstruction op = parsed_instruction_init(INS_ADD, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        if (expect(TOK_SUB)) {
            uint8_t size = 3;

            advance();

            Token dst_reg = g_current;
            match_register();

            match(TOK_COMMA);

            cvector_vector_type(uint8_t) operands = NULL;
            cvector_push_back(operands, token_to_register(dst_reg));

            if (expect(TOK_IMMEDIATE)) {
                size += sizeof(uint64_t);

                Token immediate = g_current;
                advance();

                uint64_t immediate_value = strtoul(immediate.span.data, NULL, 10);
                uint8_t* bytes = (uint8_t*)&immediate_value;

                for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                    cvector_push_back(operands, bytes[i]);

                ParsedInstruction iop = parsed_instruction_init(INS_ISUB, operands, size);
                cvector_push_back(parsed_instructions, iop);

                rip += size;
                continue;
            }

            uint8_t iteration = 0;

            do {
                if (iteration != 0 && expect(TOK_COMMA))
                    advance();

                Token src_reg = g_current;
                match_register();

                cvector_push_back(operands, token_to_register(src_reg));
                
                iteration += 1;
                size += 1;
            } while (!is_eof() && expect(TOK_COMMA));

            ParsedInstruction op = parsed_instruction_init(INS_SUB, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        if (expect(TOK_MUL)) {
            uint8_t size = 3;

            advance();

            Token dst_reg = g_current;
            match_register();

            match(TOK_COMMA);

            cvector_vector_type(uint8_t) operands = NULL;
            cvector_push_back(operands, token_to_register(dst_reg));

            if (expect(TOK_IMMEDIATE)) {
                size += sizeof(uint64_t);

                Token immediate = g_current;
                advance();

                uint64_t immediate_value = strtoul(immediate.span.data, NULL, 10);
                uint8_t* bytes = (uint8_t*)&immediate_value;

                for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                    cvector_push_back(operands, bytes[i]);

                ParsedInstruction iop = parsed_instruction_init(INS_IMUL, operands, size);
                cvector_push_back(parsed_instructions, iop);

                rip += size;
                continue;
            }

            uint8_t iteration = 0;

            do {
                if (iteration != 0 && expect(TOK_COMMA))
                    advance();

                Token src_reg = g_current;
                match_register();

                cvector_push_back(operands, token_to_register(src_reg));
                
                iteration += 1;
                size += 1;
            } while (!is_eof() && expect(TOK_COMMA));

            ParsedInstruction op = parsed_instruction_init(INS_MUL, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        if (expect(TOK_DIV)) {
            uint8_t size = 3;

            advance();

            Token dst_reg = g_current;
            match_register();

            match(TOK_COMMA);

            cvector_vector_type(uint8_t) operands = NULL;
            cvector_push_back(operands, token_to_register(dst_reg));

            if (expect(TOK_IMMEDIATE)) {
                size += sizeof(uint64_t);

                Token immediate = g_current;
                advance();

                uint64_t immediate_value = strtoul(immediate.span.data, NULL, 10);
                uint8_t* bytes = (uint8_t*)&immediate_value;

                for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                    cvector_push_back(operands, bytes[i]);

                ParsedInstruction iop = parsed_instruction_init(INS_IDIV, operands, size);
                cvector_push_back(parsed_instructions, iop);

                rip += size;
                continue;
            }

            uint8_t iteration = 0;

            do {
                if (iteration != 0 && expect(TOK_COMMA))
                    advance();

                Token src_reg = g_current;
                match_register();

                cvector_push_back(operands, token_to_register(src_reg));
                
                iteration += 1;
                size += 1;
            } while (!is_eof() && expect(TOK_COMMA));

            ParsedInstruction op = parsed_instruction_init(INS_DIV, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        if (expect(TOK_PUSH)) {
            uint8_t size = 2;

            advance();

            cvector_vector_type(uint8_t) operands = NULL;

            if (expect(TOK_IMMEDIATE)) {
                size += sizeof(uint64_t);

                Token immediate = g_current;
                advance();

                uint64_t immediate_value = strtoul(immediate.span.data, NULL, 10);
                uint8_t* bytes = (uint8_t*)&immediate_value;

                for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                    cvector_push_back(operands, bytes[i]);

                ParsedInstruction iop = parsed_instruction_init(INS_IPUSH, operands, size);
                cvector_push_back(parsed_instructions, iop);

                rip += size;
                continue;
            }

            uint8_t iteration = 0;

            do {
                if (iteration != 0 && expect(TOK_COMMA))
                    advance();

                Token src_reg = g_current;
                match_register();

                cvector_push_back(operands, token_to_register(src_reg));
                
                iteration += 1;
                size += 1;
            } while (!is_eof() && expect(TOK_COMMA));

            ParsedInstruction op = parsed_instruction_init(INS_PUSH, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        if (expect(TOK_POP)) {
            uint8_t size = 3;

            advance();

            Token dst_reg = g_current;
            match_register();

            cvector_vector_type(uint8_t) operands = NULL;
            cvector_push_back(operands, token_to_register(dst_reg));

            ParsedInstruction op = parsed_instruction_init(INS_POP, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        if (expect(TOK_MOVE)) {
            uint8_t size = 3;

            advance();

            Token dst_reg = g_current;
            match_register();

            match(TOK_COMMA);

            cvector_vector_type(uint8_t) operands = NULL;
            cvector_push_back(operands, token_to_register(dst_reg));

            if (expect(TOK_IMMEDIATE)) {
                size += sizeof(uint64_t);

                Token immediate = g_current;
                advance();

                uint64_t immediate_value = strtoul(immediate.span.data, NULL, 10);
                uint8_t* bytes = (uint8_t*)&immediate_value;

                for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                    cvector_push_back(operands, bytes[i]);

                ParsedInstruction iop = parsed_instruction_init(INS_IMOVE, operands, size);
                cvector_push_back(parsed_instructions, iop);

                rip += size;
                continue;
            }

            Token src_reg = g_current;
            match_register();
            cvector_push_back(operands, token_to_register(src_reg));

            size += 1;

            ParsedInstruction op = parsed_instruction_init(INS_MOVE, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        if (expect(TOK_CMP)) {
            uint8_t size = 3;

            advance();

            Token lhs = g_current;
            match_register();

            match(TOK_COMMA);

            cvector_vector_type(uint8_t) operands = NULL;
            cvector_push_back(operands, token_to_register(lhs));

            if (expect(TOK_IMMEDIATE)) {
                size += sizeof(uint64_t);

                Token immediate = g_current;
                advance();

                uint64_t immediate_value = strtoul(immediate.span.data, NULL, 10);
                uint8_t* bytes = (uint8_t*)&immediate_value;

                for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                    cvector_push_back(operands, bytes[i]);

                ParsedInstruction iop = parsed_instruction_init(INS_ICMP, operands, size);
                cvector_push_back(parsed_instructions, iop);

                rip += size;
                continue;
            }

            Token src_reg = g_current;
            match_register();
            cvector_push_back(operands, token_to_register(src_reg));

            size += 1;

            ParsedInstruction op = parsed_instruction_init(INS_CMP, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        if (expect(TOK_JMP)) {
            uint8_t size = 10;

            advance();

            cvector_vector_type(uint8_t) operands = NULL;

            if (expect(TOK_IDENTIFIER)) {
                Token id = g_current;
                advance();

                int index = symtab_lookup(id.span);

                if (index == -1) {
                    fprintf(stderr, "(%zu:%zu) ERROR: the lable: ", id.line, id.col);
                    span_print(stderr, id.span);
                    fprintf(stderr, "does not exist\n");

                    exit(1);
                }

                uint8_t* bytes = (uint8_t*)&g_symtab[index].ip;

                for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                    cvector_push_back(operands, bytes[i]);

                ParsedInstruction op = parsed_instruction_init(INS_JMP, operands, size);
                cvector_push_back(parsed_instructions, op);

                rip += size;
                continue;
            }

            Token immediate = g_current;
            match(TOK_IMMEDIATE);

            uint64_t immediate_value = strtoul(immediate.span.data, NULL, 10);
            uint8_t* bytes = (uint8_t*)&immediate_value;

            for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                cvector_push_back(operands, bytes[i]);

            ParsedInstruction op = parsed_instruction_init(INS_JMP, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        if (expect(TOK_JE)) {
            uint8_t size = 10;

            advance();

            cvector_vector_type(uint8_t) operands = NULL;

            if (expect(TOK_IDENTIFIER)) {
                Token id = g_current;
                advance();

                int index = symtab_lookup(id.span);

                if (index == -1) {
                    fprintf(stderr, "(%zu:%zu) ERROR: the lable: ", id.line, id.col);
                    span_print(stderr, id.span);
                    fprintf(stderr, "does not exist\n");

                    exit(1);
                }

                uint8_t* bytes = (uint8_t*)&g_symtab[index].ip;

                for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                    cvector_push_back(operands, bytes[i]);

                ParsedInstruction op = parsed_instruction_init(INS_JE, operands, size);
                cvector_push_back(parsed_instructions, op);

                rip += size;
                continue;
            }

            Token immediate = g_current;
            match(TOK_IMMEDIATE);

            uint64_t immediate_value = strtoul(immediate.span.data, NULL, 10);
            uint8_t* bytes = (uint8_t*)&immediate_value;

            for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                cvector_push_back(operands, bytes[i]);

            ParsedInstruction op = parsed_instruction_init(INS_JE, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        if (expect(TOK_JNE)) {
            uint8_t size = 10;

            advance();

            cvector_vector_type(uint8_t) operands = NULL;

            if (expect(TOK_IDENTIFIER)) {
                Token id = g_current;
                advance();

                int index = symtab_lookup(id.span);

                if (index == -1) {
                    fprintf(stderr, "(%zu:%zu) ERROR: the lable: ", id.line, id.col);
                    span_print(stderr, id.span);
                    fprintf(stderr, "does not exist\n");

                    exit(1);
                }

                uint8_t* bytes = (uint8_t*)&g_symtab[index].ip;

                for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                    cvector_push_back(operands, bytes[i]);

                ParsedInstruction op = parsed_instruction_init(INS_JNE, operands, size);
                cvector_push_back(parsed_instructions, op);

                rip += size;
                continue;
            }

            Token immediate = g_current;
            match(TOK_IMMEDIATE);

            uint64_t immediate_value = strtoul(immediate.span.data, NULL, 10);
            uint8_t* bytes = (uint8_t*)&immediate_value;

            for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                cvector_push_back(operands, bytes[i]);

            ParsedInstruction op = parsed_instruction_init(INS_JNE, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        if (expect(TOK_JG)) {
            uint8_t size = 10;

            advance();

            cvector_vector_type(uint8_t) operands = NULL;

            if (expect(TOK_IDENTIFIER)) {
                Token id = g_current;
                advance();

                int index = symtab_lookup(id.span);

                if (index == -1) {
                    fprintf(stderr, "(%zu:%zu) ERROR: the lable: ", id.line, id.col);
                    span_print(stderr, id.span);
                    fprintf(stderr, "does not exist\n");

                    exit(1);
                }

                uint8_t* bytes = (uint8_t*)&g_symtab[index].ip;

                for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                    cvector_push_back(operands, bytes[i]);

                ParsedInstruction op = parsed_instruction_init(INS_JG, operands, size);
                cvector_push_back(parsed_instructions, op);

                rip += size;
                continue;
            }

            Token immediate = g_current;
            match(TOK_IMMEDIATE);

            uint64_t immediate_value = strtoul(immediate.span.data, NULL, 10);
            uint8_t* bytes = (uint8_t*)&immediate_value;

            for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                cvector_push_back(operands, bytes[i]);

            ParsedInstruction op = parsed_instruction_init(INS_JG, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        if (expect(TOK_JL)) {
            uint8_t size = 10;

            advance();

            cvector_vector_type(uint8_t) operands = NULL;

            if (expect(TOK_IDENTIFIER)) {
                Token id = g_current;
                advance();

                int index = symtab_lookup(id.span);

                if (index == -1) {
                    fprintf(stderr, "(%zu:%zu) ERROR: the lable: ", id.line, id.col);
                    span_print(stderr, id.span);
                    fprintf(stderr, "does not exist\n");

                    exit(1);
                }

                uint8_t* bytes = (uint8_t*)&g_symtab[index].ip;

                for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                    cvector_push_back(operands, bytes[i]);

                ParsedInstruction op = parsed_instruction_init(INS_JL, operands, size);
                cvector_push_back(parsed_instructions, op);

                rip += size;
                continue;
            }

            Token immediate = g_current;
            match(TOK_IMMEDIATE);

            uint64_t immediate_value = strtoul(immediate.span.data, NULL, 10);
            uint8_t* bytes = (uint8_t*)&immediate_value;

            for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                cvector_push_back(operands, bytes[i]);

            ParsedInstruction op = parsed_instruction_init(INS_JL, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        if (expect(TOK_JGE)) {
            uint8_t size = 10;

            advance();

            cvector_vector_type(uint8_t) operands = NULL;

            if (expect(TOK_IDENTIFIER)) {
                Token id = g_current;
                advance();

                int index = symtab_lookup(id.span);

                if (index == -1) {
                    fprintf(stderr, "(%zu:%zu) ERROR: the lable: ", id.line, id.col);
                    span_print(stderr, id.span);
                    fprintf(stderr, "does not exist\n");

                    exit(1);
                }

                uint8_t* bytes = (uint8_t*)&g_symtab[index].ip;

                for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                    cvector_push_back(operands, bytes[i]);

                ParsedInstruction op = parsed_instruction_init(INS_JGE, operands, size);
                cvector_push_back(parsed_instructions, op);

                rip += size;
                continue;
            }

            Token immediate = g_current;
            match(TOK_IMMEDIATE);

            uint64_t immediate_value = strtoul(immediate.span.data, NULL, 10);
            uint8_t* bytes = (uint8_t*)&immediate_value;

            for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                cvector_push_back(operands, bytes[i]);

            ParsedInstruction op = parsed_instruction_init(INS_JGE, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        if (expect(TOK_JLE)) {
            uint8_t size = 10;

            advance();

            cvector_vector_type(uint8_t) operands = NULL;

            if (expect(TOK_IDENTIFIER)) {
                Token id = g_current;
                advance();

                int index = symtab_lookup(id.span);

                if (index == -1) {
                    fprintf(stderr, "(%zu:%zu) ERROR: the lable: ", id.line, id.col);
                    span_print(stderr, id.span);
                    fprintf(stderr, "does not exist\n");

                    exit(1);
                }

                uint8_t* bytes = (uint8_t*)&g_symtab[index].ip;

                for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                    cvector_push_back(operands, bytes[i]);

                ParsedInstruction op = parsed_instruction_init(INS_JLE, operands, size);
                cvector_push_back(parsed_instructions, op);

                rip += size;
                continue;
            }

            Token immediate = g_current;
            match(TOK_IMMEDIATE);

            uint64_t immediate_value = strtoul(immediate.span.data, NULL, 10);
            uint8_t* bytes = (uint8_t*)&immediate_value;

            for (uint8_t i = 0; i < sizeof(uint64_t); i++)
                cvector_push_back(operands, bytes[i]);

            ParsedInstruction op = parsed_instruction_init(INS_JLE, operands, size);
            cvector_push_back(parsed_instructions, op);

            rip += size;
            continue;
        }

        fprintf(stderr, "(%zu:%zu) ERROR: illegal instruction: ", g_current.line, g_current.col);
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");

        exit(1);
    }

    return parsed_instructions;
}

ParsedInstruction parsed_instruction_init(Instruction instruction, cvector_vector_type(uint8_t) operands, uint8_t size) {
    return (ParsedInstruction) {
        .instruction = instruction,
        .operands = operands,
        .size = size,
    };
}

void parsed_instruction_deinit(ParsedInstruction parsed_instruction) {
    if (parsed_instruction.operands != NULL)
        cvector_free(parsed_instruction.operands);
}
