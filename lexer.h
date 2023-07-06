#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stddef.h>

typedef struct Span_t {
    const char* data;
    size_t len;
} Span;

typedef enum TokenKind_t {
    TOK_EOF,
    TOK_HALT,
    TOK_ADD,
    TOK_SUB,
    TOK_MUL,
    TOK_DIV,
    TOK_PUSH,
    TOK_POP,
    TOK_MOVE,
    TOK_CMP,
    TOK_JMP,
    TOK_JE,
    TOK_JNE,
    TOK_JG,
    TOK_JL,
    TOK_JGE,
    TOK_JLE,
    TOK_REG_A,
    TOK_REG_B,
    TOK_REG_C,
    TOK_REG_D,
    TOK_COMMA,
    TOK_IMMEDIATE,
    TOK_LABLE,
    TOK_IDENTIFIER,
} TokenKind;

typedef struct Token_t {
    TokenKind kind;
    Span span;
    size_t line;
    size_t col;
} Token;

int lexer_init(const char* input);
Token lexer_get_token();

Span span_init(const char* data, size_t len);
Span span_from(const char* data);
Span span_init_null();
int span_equals(Span lhs, Span rhs);
void span_print(FILE* file, Span span);

Token token_init(TokenKind kind, Span span, size_t line, size_t col);

#endif /* LEXER_H */
