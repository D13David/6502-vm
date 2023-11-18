#if TOOLBUILD
#include "sml_vm.h"
#include "sml_lexer.h"

#if PLATFORM_WINDOWS
#include <intrin.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#define ADDR_MODE_IMPLICITE         0
#define ADDR_MODE_IMMEDIATE         1
#define ADDR_MODE_ABSOLUTE          2
#define ADDR_MODE_ABSOLUTE_INDEX_X  3
#define ADDR_MODE_ABSOLUTE_INDEX_Y  4
#define ADDR_MODE_ZEROPAGE          5
#define ADDR_MODE_ZEROPAGE_INDEX_X  6
#define ADDR_MODE_ZEROPAGE_INDEX_Y  7
#define ADDR_MODE_INDIRECT_INDEX_X  8
#define ADDR_MODE_INDIRECT_INDEX_Y  9
#define ADDR_MODE_RELATIVE          10
#define ADDR_MODE_UNKNOWN           99

typedef struct symbol
{
    char name[SYMBOL_NAME_SIZE_MAX];
    token_t token;
} symbol_t;

uint8_t GNumSymbols;
symbol_t GSymbols[256];
int GIsPrepass;

typedef struct mnemonic
{
    const char* value;
    const uint8_t opcodes[11];
    uint8_t flags;
} mnemonic_t;

static mnemonic_t mnemonics[] =
{
#define DEFINE_MNENOMIC(name, op0, op1, op2, op3, op4, op5, op6, op7, op8, op9, op10, f) \
    { .value=#name, \
      .opcodes={op0, op1, op2, op3, op4, op5, op6, op7, op8, op9, op10}, \
      .flags=f \
    },
#include "sml_mnemonics.h"
#undef DEFINE_MNENOMIC
};

void error(const lexer_t* lexer, const char* fmt, ...)
{
    char buffer[512];
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);
    printf("PARSER ERROR: %s (Line: %d)\n", buffer, lexer->line + 1);
    exit(0);
}

void expect_token_type(lexer_t* lexer, uint8_t type, token_t* ptoken)
{
    token_t token;
    if (lexer_read_token(lexer, &token))
    {
        if (token.type != type) {
            error(lexer, "expected token with type %d, but found %d", type, token.type);
        }
        if (ptoken) *ptoken = token;
        return;
    }
    error(lexer, "failed to read token but token was expected");
}


void expect_token_subtype(lexer_t* lexer, uint8_t type, uint8_t subtype)
{
    token_t token;
    if (lexer_read_token(lexer, &token)) 
    {
        if (token.type != type || token.flags != subtype) {
            error(lexer, "expected token with type %d, %d, but found %d, %d", type, subtype, token.type, token.flags);
        }
        return;
    }
    error(lexer, "failed to read token but token was expected");
}

void expect_token_value(lexer_t* lexer, uint8_t type, const uint8_t* value)
{
    token_t token;
    if (lexer_read_token(lexer, &token))
    {
        if (token.type != type || strncmp(token.value, value, SYMBOL_NAME_SIZE_MAX)) {
            error(lexer, "expected token with type %d, %s, but found %d, %s", type, value, token.type, token.value);
        }
        return;
    }
    error(lexer, "failed to read token but token was expected");
}

int test_token_value(lexer_t* lexer, uint8_t type, const uint8_t* value)
{
    token_t token;
    if (lexer_read_token(lexer, &token) == 0) {
        error(lexer, "failed to read token but token was expected");
    }

    int result = (token.type == type && !strncmp(token.value, value, SYMBOL_NAME_SIZE_MAX));
    if (!result) {
        lexer_undo_read(lexer);
    }
    return result;
}

int test_token_punctuation(lexer_t* lexer, uint8_t subtype)
{
    token_t token;
    if (lexer_read_token(lexer, &token) == 0) {
        error(lexer, "failed to read token but token was expected");
    }

    int result = (token.type == TT_PUNCTUATION && token.flags == subtype);
    if (!result) {
        lexer_undo_read(lexer);
    }
    return result;
}

int test_token_type(lexer_t* lexer, uint8_t type)
{
    token_t token;
    if (lexer_read_token(lexer, &token) == 0) {
        error(lexer, "failed to read token but token was expected");
    }

    int result = token.type == type;
    if (!result) {
        lexer_undo_read(lexer);
    }
    return result;
}

int parse_number_t(lexer_t* lexer, token_t token, uint16_t* value)
{
    uint16_t mask = 0xffff;
    uint16_t shift = 0;

    if (token.type == TT_PUNCTUATION) 
    {
        if (token.flags == PT_LESS) {
            mask = 0xff;
        }
        else if (token.flags == PT_MORE) {
            mask = 0xff;
            shift = 8;
        }
        expect_token_type(lexer, TT_NAME, &token);
    }

    // resolve symbolic name to value
    if (token.type == TT_NAME)
    {
        int i = 0;
        for (; i < GNumSymbols; ++i)
        {
            if (!strncmp(GSymbols[i].name, token.value, SYMBOL_NAME_SIZE_MAX))
            {
                token = GSymbols[i].token;
                break;
            }
        }

        if (!GIsPrepass && i == GNumSymbols) {
            error(lexer, "symbol %s not found", token.value);
            return 0;
        }

        // if no symbolic value was found we assume it's a label name and return some dummy value 
        // so prepass can still calculate offsets
        if (token.type == TT_NAME)
        {
            strncpy(token.value, "42", 2);
            token.len = 2;
            token.type = TT_NUMBER;
            token.flags = TTF_HEX;
        }
    }

    if (token.type != TT_NUMBER) {
        error(lexer, "expected number");
        return 0;
    }

    if (token.flags == TTF_HEX) {
        *value = (uint16_t)strtol(token.value, NULL, 16);
    }
    else {
        *value = (uint16_t)strtol(token.value, NULL, 10);
    }

    *value = (*value >> shift) & mask;

    return 1;
}

int parse_number(lexer_t* lexer, uint16_t* value)
{
    token_t token;
    if (lexer_read_token(lexer, &token) == 0) {
        return 0;
    }
    return parse_number_t(lexer, token, value);
}

int parse_parameter(lexer_t* lexer, mnemonic_t mnemonic, uint16_t* value, uint8_t* mode, uint8_t prepass)
{
    token_t token;
    if (lexer_read_token(lexer, &token) == 0) {
        *mode = ADDR_MODE_IMPLICITE;
        return 1;
    }
    else
    {
        int tokenInNextLine = lexer->linesCrossed > 0;
        lexer_undo_read(lexer);

        // no parameters
        if (tokenInNextLine) {
            *mode = ADDR_MODE_IMPLICITE;
            return 1;
        }
    }

    if (lexer_peek_punctuation(lexer, PT_OCTO))
    {
        *mode = ADDR_MODE_IMMEDIATE;
        if (parse_number(lexer, value) == 0) {
            error(lexer, "expected number as parameter");
        }
    }
    else if (lexer_peek_punctuation(lexer, PT_PARENTH_OPEN))
    {
        if (parse_number(lexer, value) == 0) {
            error(lexer, "expected number as parameter");
        }

        if (lexer_peek_punctuation(lexer, PT_PARENTH_CLOSE))
        {
            *mode = ADDR_MODE_INDIRECT_INDEX_Y;
            expect_token_subtype(lexer, TT_PUNCTUATION, PT_COMMA);
            expect_token_value(lexer, TT_NAME, "y");
        }
        else
        {
            *mode = ADDR_MODE_INDIRECT_INDEX_X;
            expect_token_subtype(lexer, TT_PUNCTUATION, PT_COMMA);
            expect_token_value(lexer, TT_NAME, "x");
            expect_token_subtype(lexer, TT_PUNCTUATION, PT_PARENTH_CLOSE);
        }
    }
    else
    {
        if (parse_number(lexer, value) == 0) {
            error(lexer, "expected number as parameter");
        }

        if (*value > 0xff)
        {
            if (lexer_peek_punctuation(lexer, PT_COMMA))
            {
                if (test_token_value(lexer, TT_NAME, "x")) {
                    *mode = ADDR_MODE_ABSOLUTE_INDEX_X;
                }
                else if (test_token_value(lexer, TT_NAME, "y")) {
                    *mode = ADDR_MODE_ABSOLUTE_INDEX_Y;
                }
                else {
                    error(lexer, "looks like absolute indexed addressing but no valid index found");
                }
            }
            else
            {
                *mode = ADDR_MODE_ABSOLUTE;
            }
        }
        else
        {
            if (lexer_peek_punctuation(lexer, PT_COMMA))
            {
                if (test_token_value(lexer, TT_NAME, "x")) {
                    *mode = ADDR_MODE_ZEROPAGE_INDEX_X;
                }
                else if (test_token_value(lexer, TT_NAME, "y")) {
                    *mode = ADDR_MODE_ZEROPAGE_INDEX_Y;
                }
                else {
                    error(lexer, "looks like absolute indexed addressing but no valid index found");
                }
            }
            else
            {
                *mode = ADDR_MODE_ZEROPAGE;
            }
        }

        // branches are always relative
        if ((mnemonic.flags & MM_BRANCH) != 0) {
            *mode = ADDR_MODE_RELATIVE;
        }

        if ((mnemonic.flags & MM_JUMP) != 0) {
            assert((*mode == ADDR_MODE_ABSOLUTE || *mode == ADDR_MODE_ZEROPAGE) && "indirect jump addressing is not supported... please implement");
            *mode = ADDR_MODE_ABSOLUTE;
        }
    }

    return 1;
}

int calculateBytesForAddressingMode(uint16_t mode)
{
    switch (mode)
    {
    case ADDR_MODE_IMPLICITE:
        return 1;
    case ADDR_MODE_ABSOLUTE:
    case ADDR_MODE_ABSOLUTE_INDEX_X:
    case ADDR_MODE_ABSOLUTE_INDEX_Y:
        return 3;
    }
    return 2;
}

int preprocess(const char* program, uint32_t size)
{
    lexer_t lexer;
    lexer_init(program, size, &lexer);

    GNumSymbols = 0;
    GIsPrepass = 1;

    uint16_t offset = 0xc000;

    while (1)
    {
        token_t token, prev;
        if (lexer_read_token(&lexer, &token) == 0) {
            break;
        }

        if (token.type == TT_PUNCTUATION && token.flags == PT_DOT)
        {
            expect_token_value(&lexer, TT_NAME, "org");
            expect_token_type(&lexer, TT_NUMBER, &token);
            parse_number_t(&lexer, token, &offset);
            continue;
        }


        if (token.type == TT_PUNCTUATION && token.flags == PT_EXCL)
        {
            int typeSize = 0;
            typeSize = lexer_peek_value(&lexer, TT_NAME, "byte") ? 1 
                : lexer_peek_value(&lexer, TT_NAME, "word") ? 2 : 0;
            if (typeSize > 0)
            {
                do
                {
                    prev = token;

                    if (lexer_read_token(&lexer, &token) == 0) {
                        break;
                    }
                    if (token.type == TT_STRING)
                    {
                        offset += token.len;
                    }
                    else
                    {
                        uint16_t value, rep = 1;
                        parse_number_t(&lexer, token, &value);

                        if (test_token_punctuation(&lexer, PT_MUL)) {
                            parse_number(&lexer, &rep);
                        }

                        offset += rep * typeSize;
                    }
                } while (lexer_peek_value(&lexer, TT_PUNCTUATION, ","));
            }
            else
            {
                error(&lexer, "unexpected value found...");
            }

            continue;
        }

        if (token.type == TT_NAME && token.flags == TTF_MNENOMIC) 
        {
            uint16_t value;
            uint8_t mode;
            if (parse_parameter(&lexer, mnemonics[token.data], &value, &mode, 1) == 0) {
                return 0;
            }

            int size = calculateBytesForAddressingMode(mode);

            //printf("%04x: %02x, %s (%d)\n", offset, mnemonics[token.data].opcodes[mode], token.value, size);

            offset = offset + size;

            continue;
        }

        if (token.type == TT_NAME && token.flags != TTF_MNENOMIC)
        {
            // FIXME: assert
            symbol_t* symbol = &GSymbols[GNumSymbols++];

            strncpy(symbol->name, token.value, SYMBOL_NAME_SIZE_MAX);

            if (lexer_read_token(&lexer, &token) == 0) {
                return 0;
            }

            if (token.type == TT_PUNCTUATION && token.value[0] == '=')
            {
                if (lexer_read_token(&lexer, &token) == 0) {
                    error(&lexer, "syntax error");
                }
                symbol->token = token;
            }
            else
            {
                lexer_undo_read(&lexer);
                lexer_reset_token(&token);
                token.type = TT_NUMBER;
                token.flags = TTF_HEX;
                sprintf(token.value, "%x", offset);
                token.len = (uint8_t)strlen(token.value);
                symbol->token = token;
            }
        }
    }

    GIsPrepass = 0;

    return 1;
}

int assemble(const char* program, uint32_t size, FILE* stream)
{
    lexer_t lexer;
    lexer_init(program, size, &lexer);

    uint16_t offset = 0xc000;

    while (1)
    {
        token_t token;
        if (lexer_read_token(&lexer, &token) == 0) {
            break;
        }

        if (token.type == TT_PUNCTUATION && token.flags == PT_DOT)
        {
            expect_token_value(&lexer, TT_NAME, "org");
            expect_token_type(&lexer, TT_NUMBER, &token);
            parse_number_t(&lexer, token, &offset);
            continue;
        }

        if (token.type == TT_PUNCTUATION && token.flags == PT_EXCL)
        {
            int typeSize = 0;
            typeSize = lexer_peek_value(&lexer, TT_NAME, "byte") ? 1
                : lexer_peek_value(&lexer, TT_NAME, "word") ? 2 : 0;
            if (typeSize > 0)
            {
                do
                {
                    if (lexer_read_token(&lexer, &token) == 0) {
                        break;
                    }
                    if (token.type == TT_STRING)
                    {
                        fwrite(token.value, sizeof(uint8_t), token.len, stream);
                        offset += token.len;
                    }
                    else
                    {
                        uint16_t value, rep = 1;
                        parse_number_t(&lexer, token, &value);
                        if (test_token_punctuation(&lexer, PT_MUL))
                        {
                            parse_number(&lexer, &rep);
                        }
                        for (int i = 0; i < rep; ++i) {
                            fwrite(&value, typeSize, 1, stream);
                        }
                        offset += rep * typeSize;
                    }
                } while (lexer_peek_value(&lexer, TT_PUNCTUATION, ","));
            }
            else
            {
                error(&lexer, "unexpected value found...");
            }

            continue;
        }

        if (token.type != TT_NAME) {
            lexer_skip_line(&lexer);
            continue;
        }

        if (token.flags == TTF_MNENOMIC)
        {
            uint16_t value;
            uint8_t mode;
            if (parse_parameter(&lexer, mnemonics[token.data], &value, &mode, 0) == 0) {
                break;
            }
            //printf("%s %02x\n", mnemonics[token.data].value, mnemonics[token.data].opcodes[mode]);

            offset = offset + calculateBytesForAddressingMode(mode);

            // generate bytecode
            if (mnemonics[token.data].opcodes[mode] == 0xff) {
                error(&lexer, "invalid opcode %s", token.value);
            }

            fwrite(&mnemonics[token.data].opcodes[mode], sizeof(uint8_t), 1, stream);

            if (mode != ADDR_MODE_IMPLICITE)
            {
                if (mode == ADDR_MODE_ABSOLUTE || mode == ADDR_MODE_ABSOLUTE_INDEX_X || mode == ADDR_MODE_ABSOLUTE_INDEX_Y)
                {
                    fwrite(&value, sizeof(uint16_t), 1, stream);
                }
                else
                {
                    if (mode == ADDR_MODE_RELATIVE)
                    {
                        int16_t jumpOffset = value - offset;
                        assert(jumpOffset >= -128 && jumpOffset <= 127 && "jump target out of range");
                        value = jumpOffset;
                    }
                    fwrite(&value, sizeof(uint8_t), 1, stream);
                }
            }
        }
    }

    return 1;
}

int compile(const char* program, uint32_t size, const char* out_name)
{
    if (!preprocess(program, size)) {
        return 0;
    }

    FILE* fp = fopen(out_name, "wb");
    if (!fp) {
        return 0;
    }

    int result = assemble(program, size, fp);

    fclose(fp);

    return result;
}

#endif