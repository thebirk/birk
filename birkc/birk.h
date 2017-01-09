#ifndef BIRK_H
#define BIRK_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// Helper macros
#define cast(type) (type)
#define ARRAYSIZE(static_array) (sizeof(static_array) / sizeof(static_array[0]))
#define BIT(n) (1<<n)
#define FOR(start, end) for(int i = start; i  < end; i++)
#define STRUCT(name) typedef struct name name; struct name
#define ENUM(name) typedef enum name name; enum name


// Typedefs
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;
typedef size_t isize;



// Array implementation originally written by Sean Barrett (https://github.com/nothings/stb/blob/master/stretchy_buffer.h)
#define birk_array_free(a)         ((a) ? free(birk_array_raw(a)),0 : 0)
#define birk_array_push(a,v)       (birk_array_maybegrow(a,1), (a)[birk_array_raw_count(a)++] = (v))
#define birk_array_count(a)        ((a) ? birk_array_raw_count(a) : 0)
#define birk_array_put(a,n)        (birk_array_maybegrow(a,n), birk_array_raw_count(a)+=(n), &(a)[birk_array_raw_count(a)-(n)])
#define birk_array_last(a)         ((a)[birk_array_raw_count(a)-1])

#define birk_array_raw(a) ((int *) (a) - 2)
#define birk_array_raw_data(a)   birk_array_raw(a)[0]
#define birk_array_raw_count(a)   birk_array_raw(a)[1]

#define birk_array_needgrow(a,n)  ((a)==0 || birk_array_raw_count(a)+(n) >= birk_array_raw_data(a))
#define birk_array_maybegrow(a,n) (birk_array_needgrow(a,(n)) ? birk_array_grow(a,n) : 0)
#define birk_array_grow(a,n)      ((a) = birk_array_grow_func((a), (n), sizeof(*(a))))

#define FOR_ARRAY(array, it) for(int it_index = 0; it = array[it_index], it_index < birk_array_count(array); it_index++)
#define FOR_ARRAY_PTR(array, it) for(int it_index = 0; it = &array[it_index], it_index < birk_array_count(array); it_index++)


// File handling
// TODO: Handle bigger than stdlib file sizes, aka use platform dependent code
STRUCT(FileData)
{
	isize size;
	u8* data;
};

FileData birk_read_entire_file(bool newline, char *path);
void birk_free_file(FileData fd);

// Text handling(ascii)

bool birk_is_uppercase(int c);
bool birk_is_lowercase(int c);
bool birk_is_alpha(int c);
bool birk_is_alnum(int c);
bool birk_is_digit(int c);

bool birk_strcmp(char *a, char *b);
bool birk_strncmp(char *a, char *b, isize n);

char* birk_strcpy(char *str);
char* birk_strncpy(char *str, isize n);



// Basic lexer
ENUM(TokenDefType)
{
	TokenDefKeyword = 0,
	TokenDefToken,
};

ENUM(BirkTokenType)
{
	TokenUnknown = 0,
	TokenNumber,
	TokenIdent,
	TokenString,
	TokenSpace,
	TokenTab,
	TokenNewline,
	TokenEof,

	BirkTokenTypeCount,
};

STRUCT(TokenDef) {
	int type;
	int id;

	char *keyword;
	char a, b, c, d;
};

STRUCT(Token)
{
	int type;
	char *text;
	isize len;
};

STRUCT(Lexer)
{
	TokenDef *token_defs;
	isize def_count;
	char *data;
	isize data_size;
	bool eat_whitespace;
	bool parse_string;

	char *curr, *end;
};

ENUM(TokenDefsResult)
{
	TokenDefsValid = 0,
	TokenDefsInvalidDefType,
	TokenDefsInvalidKeyword,
	TokenDefsInvalidToken,
};

TokenDefsResult birk_lexer_validate_token_defs(TokenDef *token_defs, isize def_count, int* index);
void birk_lexer_init(Lexer *l, char *data, isize data_size, TokenDef *token_defs, isize def_count, bool eat_whitespace, bool parse_string);
void birk_lexer_eat_whitespace(Lexer *l);
Token birk_lexer_get_token(Lexer *l);

#endif /* BIRK_H */

#ifdef BIRK_IMPLEMENTATION

static void * birk_array_grow_func(void *arr, int increment, int itemsize)
{
   int dbl_cur = arr ? 2*birk_array_raw_data(arr) : 0;
   int min_needed = birk_array_count(arr) + increment;
   int m = dbl_cur > min_needed ? dbl_cur : min_needed;
   int *p = (int *) realloc(arr ? birk_array_raw(arr) : 0, itemsize * m + sizeof(int)*2);
   if (p) {
      if (!arr)
         p[1] = 0;
      p[0] = m;
      return p+2;
   } else {
      return (void *) (2*sizeof(int)); // try to force a null pointer exception later
   }
}

FileData birk_read_entire_file(bool zero, char *path)
{
	FileData fd = {0};

	if(path == 0) {
		return fd;
	}

	FILE *f = fopen(path, "rb");
	if(!f) {
		return fd;
	}

	fseek(f, 0, SEEK_END);
	fd.size = ftell(f);
	rewind(f);

	if(zero) {
		fd.data = malloc(fd.size+1);
	} else {
		fd.data = malloc(fd.size);
	}
	
	fread(fd.data, 1, fd.size, f);
	fclose(f);
	if(zero) {
		fd.data[fd.size] = 0;
	}

	return fd;
}

void birk_free_file(FileData fd)
{
	free(fd.data);
}

bool birk_is_uppercase(int c) { return ((c >= 'A') && (c <= 'Z')); }
bool birk_is_lowercase(int c) { return ((c >= 'a') && (c <= 'z')); }
bool birk_is_alpha(int c) { return birk_is_lowercase(c) || birk_is_uppercase(c); }
bool birk_is_alnum(int c) { return birk_is_alpha(c) || birk_is_digit(c); }
bool birk_is_digit(int c) { return ((c >= '0' && (c <= '9'))); }

bool birk_strcmp(char *a, char *b)
{
	while(*a && (*a == *b)) {
		a++;
		b++;
	}
	return ( *cast(unsigned char*)a - *cast(unsigned char*)b );
}

bool birk_strncmp(char *a, char *b, isize n)
{
	while(n && *a && (*a == *b)) {
		++a;
		++b;
		--n;
	}

	if(n == 0) {
		return 0;
	} else {
		return ( *cast(unsigned char*)a - *cast(unsigned char*)b );
	}
}

char* birk_strcpy(char *str)
{
	isize len = strlen(str);
	char *result = malloc(len+1);
	memcpy(result, str, len);
	result[len] = 0;
	return result;
}

char* birk_strncpy(char *str, isize n)
{
	char *result = malloc(n+1);
	memcpy(result, str, n);
	result[n] = 0;
	return result;
}

// Param: index - The index when the function returned
TokenDefsResult birk_lexer_validate_token_defs(TokenDef *token_defs, isize def_count, int* index)
{
	FOR(0, def_count) {
		TokenDef td = token_defs[i];

		switch(td.type) {
			case TokenDefKeyword: {
				if(td.keyword == 0) {
					if(index != 0) *index = i;
					return TokenDefsInvalidKeyword;
				}
			} break;
			case TokenDefToken: {
				if(td.a == 0) {
					if(index != 0) *index = i;
					return TokenDefsInvalidToken;
				}
			} break;

			default: {
				if(index != 0) *index = i;
				return TokenDefsInvalidDefType;
			} break;
		}
	}

	if(index != 0) *index = def_count-1;
	return TokenDefsValid;
}

void birk_lexer_init(Lexer *l, char *data, isize data_size, TokenDef *token_defs, isize def_count, bool eat_whitespace, bool parse_string)
{
	l->data = data;
	l->data_size = data_size;
	l->token_defs = token_defs;
	l->def_count = def_count;
	l->eat_whitespace = eat_whitespace;
	l->parse_string = parse_string;

	l->curr = data;
	l->end = data + data_size;
}

void birk_lexer_eat_whitespace(Lexer *l)
{	
	for(;;) {
		switch(*l->curr) {
			case ' ':
			case '\t':
			case '\r':
			case '\n': {
				l->curr++;
				continue;
			} break;

			default: {
				return;
			}
		}
	}
}

Token birk_lexer_get_token(Lexer *l)
{
	if(l->eat_whitespace) birk_lexer_eat_whitespace(l);

	if(l->curr >= l->end) {
		return cast(Token) {TokenEof, 0, 0};
	}

	char c = *l->curr;

	if(!l->eat_whitespace) {
		switch(c) {
			case '\r': {
				l->curr++;
				return birk_lexer_get_token(l);
			} break;
			case ' ': {
				l->curr++;
				return cast(Token){TokenSpace, l->curr-1, 1};
			} break;
			case '\t': {
				l->curr++;
				return cast(Token){TokenTab, l->curr-1, 1};
			} break;
			case '\n': {
				l->curr++;
				return cast(Token){TokenNewline, l->curr-1, 1};
			} break;
		}
	}
	
	FOR(0, l->def_count) {
		TokenDef td = l->token_defs[i];

		if(td.type == TokenDefToken) {
			if(td.b == 0) {
				if(c == td.a) {
					l->curr++;
					return cast(Token){td.id, l->curr-1, 1};
				}
			} else if(td.c == 0) {
				// Double token
				if(c == td.a &&
					(l->curr+1 < l->end) && (*(l->curr+1) == td.b)) {
					l->curr += 2;
					return cast(Token){td.id, l->curr-2, 2};
				}
			} else if(td.d == 0) {
				// Triple token
				if(c == td.a &&
					(l->curr+1 < l->end) && (*(l->curr+1) == td.b) &&
					(l->curr+2 < l->end) && (*(l->curr+2) == td.c)) {
					l->curr += 3;
					return cast(Token){td.id, l->curr-3, 3};
				}
			} else {
				// Quad token
				if(c == td.a &&
					(l->curr+1 < l->end) && (*(l->curr+1) == td.b) &&
					(l->curr+2 < l->end) && (*(l->curr+2) == td.c) &&
					(l->curr+3 < l->end) && (*(l->curr+3) == td.d)) {
					l->curr += 4;
					return cast(Token){td.id, l->curr-4, 4};
				}
			}
		}
	}

	if(birk_is_alpha(c) || c == '_') {
		Token t = {0};
		t.type = TokenIdent;
		t.text = l->curr;
		t.len = 1;

		c = *(++l->curr);
		while(birk_is_alnum(c) || c == '_') {
			t.len++;
			c = *(++l->curr);
		}

		FOR(0, l->def_count) {
			TokenDef td = l->token_defs[i];

			if(td.type == TokenDefKeyword) {
				if(birk_strncmp(t.text, td.keyword, t.len) == 0) {
					t.type = td.id;
					return t;
				}
			}
		}

		return t;
	}

	if(birk_is_digit(c) || c == '.') {
		bool found_dot = false;
		if(c == '.') found_dot = true;

		Token t = {0};
		t.type = TokenNumber;
		t.text = l->curr;
		t.len = 1;

		c = *(++l->curr);
		while(birk_is_digit(c) || c == '.') {
			if(c == '.') {
				if(found_dot) {
					return t;
				} else {
					found_dot = true;
				}
			}

			t.len++;
			c = *(++l->curr);
		}
		//l->curr++;

		return t;
	}

	if(c == '"') {
		Token t = {0};
		t.type = TokenString;
		t.text = l->curr;
		t.len = 1;

		c = *(++l->curr);
		while(c != '"') {
			t.len++;
			c = *(++l->curr);
		}
		t.len++;
		l->curr += 1;

		return t;
	}

	l->curr++;
	return cast(Token){TokenUnknown, l->curr-1, 1};
}

#endif /* BIRK_IMPLEMENTATION */