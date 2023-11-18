#if TOOLBUILD
#include "sml_lexer.h"

#include <string.h>

#if PLATFORM_WINDOWS
#define stricmp _stricmp
#elif PLATFORM_LINUX
#define stricmp strcasecmp
#endif

#define SYMBOL_VALUE_SIZE_MAX       64

typedef struct punctuation
{
    const char* value;
    uint8_t type;
} punctuation_t;

punctuation_t punctuations[] =
{
    {"#", PT_OCTO},
    {",", PT_COMMA},
    {"!", PT_EXCL},
    {"=", PT_ASSIGN},
    {"<", PT_LESS},
    {">", PT_MORE},
    {"(", PT_PARENTH_OPEN},
    {")", PT_PARENTH_CLOSE},
    {"*", PT_MUL},
    {".", PT_DOT},
    {NULL, 0}
};

typedef struct mnemonic
{
    const char* value;
} mnemonic_t;

static mnemonic_t mnemonics[] =
{
#define DEFINE_MNENOMIC(name, op0, op1, op2, op3, op4, op5, op6, op7, op8, op9, op10, flags) {#name},
#include "sml_mnemonics.h"
#undef DEFINE_MNENOMIC
};

int lexer_read_token(lexer_t*, token_t*);

void lexer_init(const char* program, uint32_t size, lexer_t* lexer)
{
    lexer->ptr = program;
    lexer->end = program + size;
    lexer->prevPtr = lexer->ptr;
    lexer->line = lexer->prevLine = 0;
    lexer->linesCrossed = 0;
    lexer->size = size;
}

void lexer_reset_token(token_t* token)
{
    token->type = TT_UNKNOWN;
    token->flags = 0;
    token->len = 0;
    token->value[token->len] = '\0';
}

void lexer_undo_read(lexer_t* lexer)
{
    lexer->ptr = lexer->prevPtr;
    lexer->line = lexer->prevLine;
    lexer->linesCrossed = 0;
}

int lexer_skip_line(lexer_t* lexer)
{
    while (*lexer->ptr != '\n' && lexer->ptr != lexer->end)
    {
        lexer->ptr++;
    }
    return 1;
}

int lexer_skip_whitespaces(lexer_t* lexer)
{
    while (1)
    {
        if (lexer->ptr == lexer->end) {
            return 0;
        }

        while (*lexer->ptr <= ' ')
        {
            if (lexer->ptr == lexer->end || !*lexer->ptr) {
                return 0;
            }

            if (*lexer->ptr == '\n') {
                lexer->line++;
            }

            lexer->ptr++;
        }

        if (*lexer->ptr == ';')
        {
            do
            {
                if (lexer->ptr == lexer->end || !*lexer->ptr) {
                    return 0;
                }

                lexer->ptr++;
            } while (*lexer->ptr != '\n');

            continue;
        }

        break;
    }

    return 1;
}

int lexer_check_value(lexer_t* lexer, uint16_t type, const char* value)
{
    token_t token;
    if (lexer_read_token(lexer, &token) == 0) {
        return -1;
    }

    if (token.type == type && !strncmp(token.value, value, SYMBOL_NAME_SIZE_MAX)) {
        return 1;
    }

    return 0;
}

int lexer_check_punctuation(lexer_t* lexer, uint8_t punctuation)
{
    token_t token;
    if (lexer_read_token(lexer, &token) == 0) {
        return -1;
    }

    if (token.type == TT_PUNCTUATION && token.flags == punctuation) {
        return 1;
    }

    return 0;
}

int lexer_peek_value(lexer_t* lexer, uint16_t type, const char* value)
{
    int result = lexer_check_value(lexer, type, value);
    if (result <= 0)
    {
        if (result != -1) lexer_undo_read(lexer);
        return 0;
    }
    return 1;
}

int lexer_peek_punctuation(lexer_t* lexer, uint8_t punctuation)
{
    int result = lexer_check_punctuation(lexer, punctuation);
    if (result <= 0)
    {
        if (result != -1) lexer_undo_read(lexer);
        return 0;
    }
    return 1;
}

int lexer_read_name(lexer_t* lexer, token_t* token)
{
    lexer_reset_token(token);

    token->type = TT_NAME;

    do
    {
        token->value[token->len++] = *lexer->ptr++;
    } while ((*lexer->ptr >= 'a' && *lexer->ptr <= 'z') ||
        (*lexer->ptr >= 'A' && *lexer->ptr <= 'Z') ||
        (*lexer->ptr >= '0' && *lexer->ptr <= '9') || *lexer->ptr == '_');

    token->value[token->len] = '\0';

    for (int i = 0; i < sizeof(mnemonics) / sizeof(mnemonics[0]); ++i)
    {
        if (!stricmp(mnemonics[i].value, token->value))
        {
            token->flags = TTF_MNENOMIC;
            token->data = i;
            break;
        }
    }

    return 1;
}

int lexer_read_number(lexer_t* lexer, token_t* token)
{
    lexer_reset_token(token);

    if (*lexer->ptr == '$')
    {
        lexer->ptr++;

        while ((*lexer->ptr >= '0' && *lexer->ptr <= '9') || (*lexer->ptr >= 'a' && *lexer->ptr <= 'f') || (*lexer->ptr >= 'A' && *lexer->ptr <= 'F'))
        {
            token->value[token->len++] = *lexer->ptr++;
        }

        token->flags = TTF_HEX;
    }
    else
    {
        while ((*lexer->ptr >= '0' && *lexer->ptr <= '9'))
        {
            token->value[token->len++] = *lexer->ptr++;
        }

        token->flags = TTF_DEC;
    }

    token->type = TT_NUMBER;
    token->value[token->len] = '\0';

    return 1;
}

int lexer_read_string(lexer_t* lexer, token_t* token)
{
    lexer_reset_token(token);

    lexer->ptr++;

    while (*lexer->ptr != '\0' && lexer->ptr != lexer->end)
    {
        if (*lexer->ptr == '\"') {
            lexer->ptr++;
            break;
        }

        token->value[token->len++] = *lexer->ptr++;
    }

    token->type = TT_STRING;
    token->value[token->len] = '\0';

    return 1;
}

int lexer_read_punctuation(lexer_t* lexer, token_t* token)
{
    lexer_reset_token(token);

    for (int i = 0; punctuations[i].value; ++i)
    {
        const punctuation_t* punctuation = &punctuations[i];

        const char* value = punctuation->value;
        int j = 0;
        for (; value[j] && lexer->ptr[j]; ++j)
        {
            if (lexer->ptr[j] != value[j]) {
                break;
            }

            token->value[token->len++] = lexer->ptr[j];
        }

        if (!value[j])
        {
            token->type = TT_PUNCTUATION;
            token->flags = punctuation->type;
            lexer->ptr += token->len;
            break;
        }
        else
        {
            token->len = 0;
        }
    }

    token->value[token->len] = '\0';
    return token->len > 0;
}

int lexer_read_token(lexer_t* lexer, token_t* token)
{
    if (!lexer_skip_whitespaces(lexer)) {
        return 0;
    }

    lexer->linesCrossed = lexer->line - lexer->prevLine;
    lexer->prevPtr = lexer->ptr;
    lexer->prevLine = lexer->line;

    uint8_t c = *lexer->ptr;

    if (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
    {
        lexer_read_name(lexer, token);
    }
    else if ((c >= '0' && c <= '9') || c == '$')
    {
        lexer_read_number(lexer, token);
    }
    else if (c == '"')
    {
        lexer_read_string(lexer, token);
    }
    else
    {
        lexer_read_punctuation(lexer, token);
    }

    return 1;
}
#endif