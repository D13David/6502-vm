#pragma once

#if TOOLBUILD

#include <stdint.h>

// token type
#define TT_NAME                     0
#define TT_PUNCTUATION              1
#define TT_NUMBER                   2
#define TT_STRING                   3
#define TT_UNKNOWN                  0xff

// token subtype, set in token flags when token is other than TT_PUNCTUATION
#define TTF_HEX                     0
#define TTF_DEC                     1
#define TTF_MNENOMIC                2

// punctuation types, set in token flags when token is TT_PUNCTUATION
#define PT_OCTO                     0
#define PT_COMMA                    1
#define PT_EXCL                     2
#define PT_ASSIGN                   3
#define PT_LESS                     4
#define PT_MORE                     5
#define PT_PARENTH_OPEN             6
#define PT_PARENTH_CLOSE            7
#define PT_MUL                      8
#define PT_DOT                      9

#define SYMBOL_NAME_SIZE_MAX        64

typedef struct token
{
    char value[SYMBOL_NAME_SIZE_MAX];
    uint8_t len;
    uint8_t type;
    uint16_t flags;
    uint8_t data;
} token_t;

typedef struct lexer_s
{
    const char* ptr;
    const char* end;
    const char* prevPtr;
    uint16_t line;
    uint16_t prevLine;
    uint16_t linesCrossed;
    uint32_t size;
} lexer_t;

void lexer_init(const char* program, uint32_t size, lexer_t* lexer);
int lexer_read_token(lexer_t* lexer, token_t* token);
void lexer_undo_read(lexer_t* lexer);
int lexer_check_value(lexer_t* lexer, uint16_t type, const char* value);
int lexer_peek_value(lexer_t* lexer, uint16_t type, const char* value);
int lexer_check_punctuation(lexer_t* lexer, uint8_t punctuation);
int lexer_peek_punctuation(lexer_t* lexer, uint8_t punctuation);
void lexer_reset_token(token_t* token);
int lexer_skip_line(lexer_t* lexer);

#endif