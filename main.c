#define BIRK_IMPLEMENTATION
#include "birk.h"

void array_test()
{
	int *array = 0;

	birk_array_push(array, 123);
	birk_array_push(array, 321);
	birk_array_push(array, 123321);

	int it;
	FOR_ARRAY(array, it) {
		printf("%d: %d\n", it_index, it);
	}
	birk_array_free(array);
}

void file_test()
{
	FileData fd = birk_read_entire_file(true, "main.c");
	printf("%s\n", fd.data);
	birk_free_file(fd);
}

ENUM(TokenType)
{
	TokenOpenPar = BirkTokenTypeCount,
	TokenClosePar,
	TokenOpenBracket,
	TokenCloseBracket,
	TokenOpenBrace,
	TokenCloseBrace,

	TokenVoid,
	TokenChar,
	TokenShort,
	TokenInt,

	TokenFloat,
	TokenDouble,
	TokenUnsigned,
	TokenSigned,
	TokenReturn,
	TokenSizeof,

	TokenAsterisk,
	TokenSlash,
	TokenPlus,
	TokenMinus,
	TokenPercent,
	TokenEqual,
	TokenSemicolon,
	TokenHash,
	TokenComma,

	TokenGTE,
	TokenTripleShift,
	TokenWTF,
};

TokenDef token_defs[] = {
	{TokenDefToken, TokenOpenPar, 0, '(', 0, 0, 0},
	{TokenDefToken, TokenClosePar, 0, ')', 0, 0, 0},
	{TokenDefToken, TokenOpenBracket, 0, '[', 0, 0, 0},
	{TokenDefToken, TokenCloseBracket, 0, ']', 0, 0, 0},
	{TokenDefToken, TokenOpenBrace, 0, '{', 0, 0, 0},
	{TokenDefToken, TokenCloseBrace, 0, '}', 0, 0, 0},

	{TokenDefKeyword, TokenVoid, "void", 0, 0, 0, 0},
	{TokenDefKeyword, TokenChar, "char", 0, 0, 0, 0},
	{TokenDefKeyword, TokenShort, "short", 0, 0, 0, 0},
	{TokenDefKeyword, TokenInt, "int", 0, 0, 0, 0},
	{TokenDefKeyword, TokenFloat, "float", 0, 0, 0, 0},
	{TokenDefKeyword, TokenDouble, "double", 0, 0, 0, 0},
	{TokenDefKeyword, TokenUnsigned, "unsigned", 0, 0, 0, 0},
	{TokenDefKeyword, TokenSigned, "signed", 0, 0, 0, 0},
	{TokenDefKeyword, TokenReturn, "return", 0, 0, 0, 0},
	{TokenDefKeyword, TokenSizeof, "sizeof", 0, 0, 0, 0},

	{TokenDefToken, TokenAsterisk, 0, '*', 0, 0, 0},
	{TokenDefToken, TokenSlash, 0, '/', 0, 0, 0},
	{TokenDefToken, TokenPlus, 0, '+', 0, 0, 0},
	{TokenDefToken, TokenMinus, 0, '-', 0, 0, 0},
	{TokenDefToken, TokenPercent, 0, '%', 0, 0, 0},
	{TokenDefToken, TokenEqual, 0, '=', 0, 0, 0},
	{TokenDefToken, TokenSemicolon, 0, ';', 0, 0, 0},
	{TokenDefToken, TokenHash, 0, '#', 0, 0, 0},
	{TokenDefToken, TokenComma, 0, ',', 0, 0, 0},

	{TokenDefToken, TokenGTE, 0, '>', '=', 0, 0},
	{TokenDefToken, TokenTripleShift, 0, '>', '>', '>', 0},
	{TokenDefToken, TokenWTF, 0, '<', '=', '!', '>'},
};

void lexer_test()
{
	FileData fd = birk_read_entire_file(false, "lexer_test.c");

	int index = 0;
	TokenDefsResult state = birk_lexer_validate_token_defs(token_defs, ARRAYSIZE(token_defs), &index);
	if(state != TokenDefsValid) {
		printf("Invalid token definitions: %d, index: %d\n", state, index);
		return;
	}

	Lexer l = {0};
	birk_lexer_init(&l, cast(char*)fd.data, fd.size, token_defs, ARRAYSIZE(token_defs), true, true);

	Token t = birk_lexer_get_token(&l);
	
	while(t.type != TokenEof) {
		printf("%d: %.*s\n", t.type, cast(int)t.len, t.text);

		t = birk_lexer_get_token(&l);
	}

	birk_free_file(fd);
}

#ifdef BIRK_HAS_FBLOCKS
void defer_test()
{
	defer { printf(", World!\n"); };
	printf("Hello");
}
#endif

int main()
{
	defer_test();

	return 0;
}
