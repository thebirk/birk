# birk.h

## What is this?

This is my header that I include in almost all projects I work on. It contains several macros and small "libraries" that I use from time to time.

This header is not compiler extension shy and it contains a handfull of cool macros like this cool defer macro.

```c
int main()
{
	defer { printf(", world!\n"); };
	printf("Hello");

	return 0;
}
```

## How to use

If you just include the header you will get some macros and some functions signatures.
To be able to use the functions you have to put `#define BIRK_IMPLEMENTATION` before including the header but only once. Otherwise you will get error about trying to redefine functions.

## Features list
Here is a full list of all "features"

- [Helper macros](#helper-macros)
- [Typedefs](#typedefs)
- [Defer statement](#defer-statement)
- [Array](#array)
- [FileData](#filedata)
- [Lexer](#lexer)

## Features

### Helper macros

`#define cast(type) (type)`
Basically an empty macro to help with searching for casts
`#define ARRAYSIZE(static_array) (sizeof(static_array) / sizeof(static_array[0]))`
You know what this does.
`#define BIT(n) (1<<n)`
You also know what this does.
`#define FOR(start, end) for(int i = start; i  < end; i++)`
This doesn't really need an explenation.

`#define STRUCT(name) typedef struct name name; struct name`
`#define ENUM(name) typedef enum name name; enum name`
`#define PACKED_STRUCT(name) typedef struct name name; struct __attribute__((packed)) name`
Make it easier to read a c struct typedef. Also PACKED_STRUCT can be ifdeffed to something different if the compiler doesnt support attr packed

### Typedefs
```c
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;
typedef size_t isize;
```

### Defer statement

This is a really hacky macro.
Depends the clang `__attribute__((cleanup()))` extension and also the compiler-rt BlocksRuntime. Both of these are supported by clang and some versions of gcc.

I am not sure how much of a overhead these defers actually have. Seeing as they are a leftover from Apple during their Objective-C time, it's likely that is has some severe overhead.

```c
int main()
{
	defer { printf(", world!\n"); };
	printf("Hello");

	return 0;
}
```

### Array

This array implementation is based on Sean Barrett's https://github.com/nothings/stb/blob/master/stretchy_buffer.h More of a direct rippoff really. Go check him out! He has tons of public domain libraries including stb_image.h and stb_truetype.h which are truly amazing.

```c
int main()
{
	int *array = 0;
	defer { birk_array_free(array);};

	birk_array_push(array, 123);
	birk_array_push(array, 321);
	birk_array_push(array, 123321);

	int it;
	FOR_ARRAY(array, it) {
		printf("%d: %d\n", it_index, it);
	}

	return 0;
}
```

### FileData

Simple file reading structure

```c
STRUCT(FileData)
{
	isize size;
	u8* data;
};

FileData birk_read_entire_file(bool newline, char *path);
void birk_free_file(FileData fd);
```

### Lexer

A simple lexer that allows you to supply it with custom keywords and tokens. Useful for quickly parsing some text. Can report spaces, tabs and newlines if requested.

```c
ENUM(TokenType)
{
	TokenOpenPar = BirkTokenTypeCount,
	TokenClosePar,
	TokenName,
};

TokenDef token_defs[] = {
	{TokenDefToken, TokenOpenPar, 0, '(', 0, 0, 0},
	{TokenDefToken, TokenClosePar, 0, ')', 0, 0, 0},
	{TokenDefKeyword, TokenName, "Name", 0, 0, 0, 0},
};

int main()
{
	FileData fd = birk_read_entire_file(false, "file_to_lex.txt");
	defer { birk_free_file(fd); };

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

	return 0;
}
```