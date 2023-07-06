#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include "lexer.h"

static const char* g_input;
static size_t g_col;
static size_t g_line;

static int is_eof() {
    return *g_input == 0;
}

static void advance() {
    if (is_eof())
        return;

    if (*g_input == '\n') {
        g_line++;
        g_col = 1;
    } else {
        g_col++;
    }

    g_input++;
}

static void skip_whitespaces() {
    while (!is_eof() && isspace(*g_input))
        advance();
}

static void skip_comments() {
    if (*g_input == ';') {
        while (!is_eof() && *g_input != '\n')
            advance();

        skip_whitespaces();
    }
}

int lexer_init(const char* input) {
    if (!input)
        return 0;

    g_input = input;
    g_col = 1;
    g_line = 1;

    return 1;
}

Token lexer_get_token() {
    skip_whitespaces();
    skip_comments();

    const char* current = g_input;
    size_t line = g_line;
    size_t col = g_col;

    if (is_eof())
        return token_init(TOK_EOF, span_init_null(), line, col);

    switch (*current) {
    case ',':
        advance();
        return token_init(TOK_COMMA, span_from(","), line, col);
    default:
        break;
    }

    if (isalpha(*current)) {
        size_t len = 0;

        do {
            len++;
            advance();
        } while (!is_eof() && isalpha(*g_input));

        Span span = span_init(current, len);

        if (span_equals(span, span_from("halt"))) {
            return token_init(TOK_HALT, span, line, col);
        } else if (span_equals(span, span_from("add"))) {
            return token_init(TOK_ADD, span, line, col);
        } else if (span_equals(span, span_from("sub"))) {
            return token_init(TOK_SUB, span, line, col);
        } else if (span_equals(span, span_from("mul"))) {
            return token_init(TOK_MUL, span, line, col);
        } else if (span_equals(span, span_from("div"))) {
            return token_init(TOK_DIV, span, line, col);
        } else if (span_equals(span, span_from("push"))) {
            return token_init(TOK_PUSH, span, line, col);
        } else if (span_equals(span, span_from("pop"))) {
            return token_init(TOK_POP, span, line, col);
        } else if (span_equals(span, span_from("move"))) {
            return token_init(TOK_MOVE, span, line, col);
        } else if (span_equals(span, span_from("cmp"))) {
            return token_init(TOK_CMP, span, line, col);
        } else if (span_equals(span, span_from("jmp"))) {
            return token_init(TOK_JMP, span, line, col);
        } else if (span_equals(span, span_from("je"))) {
            return token_init(TOK_JE, span, line, col);
        } else if (span_equals(span, span_from("jne"))) {
            return token_init(TOK_JNE, span, line, col);
        } else if (span_equals(span, span_from("jg"))) {
            return token_init(TOK_JG, span, line, col);
        } else if (span_equals(span, span_from("jl"))) {
            return token_init(TOK_JL, span, line, col);
        } else if (span_equals(span, span_from("jge"))) {
            return token_init(TOK_JGE, span, line, col);
        } else if (span_equals(span, span_from("jle"))) {
            return token_init(TOK_JLE, span, line, col);
        } else if (span_equals(span, span_from("RA"))) {
            return token_init(TOK_REG_A, span, line, col);
        } else if (span_equals(span, span_from("RB"))) {
            return token_init(TOK_REG_B, span, line, col);
        } else if (span_equals(span, span_from("RC"))) {
            return token_init(TOK_REG_C, span, line, col);
        } else if (span_equals(span, span_from("RD"))) {
            return token_init(TOK_REG_D, span, line, col);
        } else {
            if (*g_input == ':') {
                advance();
                return token_init(TOK_LABLE, span, line, col);
            }

            return token_init(TOK_IDENTIFIER, span, line, col);
        }
    }

    if (isdigit(*current)) {
        size_t len = 0;

        do {
            len++;
            advance();
        } while (!is_eof() && isdigit(*g_input));

        Span span = span_init(current, len);

        strtoull(current, NULL, 10);

        if (errno == ERANGE) {
            fprintf(stderr, "(%zu:%zu) ERROR: ", line, col);
            span_print(stderr, span);
            fprintf(stderr, ", immediate too large\n");

            exit(1);
        }

        return token_init(TOK_IMMEDIATE, span, line, col);
    }

    fprintf(stderr, "(%zu:%zu) ERROR: illegal token: %c\n", g_line, g_col, *g_input);
    exit(1);
}

Span span_init(const char* data, size_t len) {
    return (Span) {
        .data = data,
        .len = len,
    };
}

Span span_from(const char* data) {
    size_t len = 0;

    while (data[len])
        len++;

    return (Span) {
        .data = data,
        .len = len,
    };
}

Span span_init_null() {
    return (Span) {
        .data = NULL,
        .len = 0,
    };
}

int span_equals(Span lhs, Span rhs) {
    if (lhs.len != rhs.len)
        return 0;

    for (size_t i = 0; i < lhs.len; i++) {
        if (lhs.data[i] != rhs.data[i])
            return 0;
    }

    return 1;
}

void span_print(FILE* file, Span span) {
    for (size_t i = 0; i < span.len; i++)
        fprintf(file, "%c", span.data[i]);
}

Token token_init(TokenKind kind, Span span, size_t line, size_t col) {
    return (Token) {
        .kind = kind,
        .span = span,
        .line = line,
        .col = col,
    };
}
