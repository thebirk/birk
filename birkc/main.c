#define BIRK_IMPLEMENTATION
#include "birk.h"

ENUM(TokenType)
{
	TokenBase = BirkTokenTypeCount,
	TokenChild,
	TokenIs,
	TokenOpenBrace,
	TokenCloseBrace,
	TokenSemicolon,
};

TokenDef token_defs[] = {
	{TokenDefKeyword, TokenBase, "base", 0, 0, 0, 0},
	{TokenDefKeyword, TokenChild, "child", 0, 0, 0, 0},
	{TokenDefToken, TokenIs, 0, ':', 0, 0, 0},
	{TokenDefToken, TokenOpenBrace, 0, '{', 0, 0, 0},
	{TokenDefToken, TokenCloseBrace, 0, '}', 0, 0, 0},
	{TokenDefToken, TokenSemicolon, 0, ';', 0, 0, 0},
};

STRUCT(ChildStruct)
{
	char *name;
	char *parent_name;
	Token **members;
};

STRUCT(BaseStruct)
{
	char *name;
	Token **members;
	ChildStruct *children;
};

int main(int argc, char **argv)
{
	if(argc != 2) {
		printf("Usage: birkc <file>\n");
		return 1;
	}

	int index = 0;
	TokenDefsResult result = birk_lexer_validate_token_defs(token_defs, ARRAYSIZE(token_defs), &index);
	if(result != TokenDefsValid) {
		printf("TokenDefs not valid: %d at %d\n", result, index);
		return 1;
	}	

	FileData fd = birk_read_entire_file(false, argv[1]);
	Lexer l = {0};
	birk_lexer_init(&l, fd.data, fd.size, token_defs, ARRAYSIZE(token_defs), false, true);

	BaseStruct *base_structs = 0;
	ChildStruct *child_structs = 0;
	Token *rest_of_file = 0;

#define LEAVE(i) do {birk_free_file(fd); return i;} while(0)
#define EXPECT(typ) do {t = birk_lexer_get_token(&l); if(t.type != typ) { printf("Expected %d!\n", typ); LEAVE(1); }} while(0)

	Token t = birk_lexer_get_token(&l);
	while(t.type != TokenEof) {
		if(t.type == TokenBase) {
			birk_lexer_eat_whitespace(&l);

			Token name = birk_lexer_get_token(&l);
			if(name.type != TokenIdent) {
				printf("Expected name after 'base'!\n");
				LEAVE(1);
			}

			birk_lexer_eat_whitespace(&l);
			EXPECT(TokenOpenBrace);

			BaseStruct base = {0};
			base.name = birk_strncpy(name.text, name.len);
			base.children = 0;
			base.members = 0;

			birk_lexer_eat_whitespace(&l);
			t = birk_lexer_get_token(&l);
			while(t.type != TokenCloseBrace) {
				Token *member = 0;
				while(t.type != TokenSemicolon) {
					birk_array_push(member, t);
					birk_lexer_eat_whitespace(&l);
					t = birk_lexer_get_token(&l);
				}
				birk_array_push(base.members, member);
				birk_lexer_eat_whitespace(&l);
				t = birk_lexer_get_token(&l);
			}
			birk_lexer_eat_whitespace(&l);
			EXPECT(TokenSemicolon);

			birk_array_push(base_structs, base);

			birk_lexer_eat_whitespace(&l);
			t = birk_lexer_get_token(&l);
			continue;

		} else if(t.type == TokenChild) {
			birk_lexer_eat_whitespace(&l);
			Token name = birk_lexer_get_token(&l);
			if(name.type != TokenIdent) {
				printf("Expected name after 'base'!\n");
				LEAVE(1);
			}

			birk_lexer_eat_whitespace(&l);
			EXPECT(TokenIs);

			birk_lexer_eat_whitespace(&l);
			Token parent = birk_lexer_get_token(&l);
			if(name.type != TokenIdent) {
				printf("Expected parent name after 'is'!\n");
				LEAVE(1);
			}

			birk_lexer_eat_whitespace(&l);
			EXPECT(TokenOpenBrace);

			ChildStruct child = {0};
			child.name = birk_strncpy(name.text, name.len);
			child.parent_name = birk_strncpy(parent.text, parent.len);
			child.members = 0;

			birk_lexer_eat_whitespace(&l);
			t = birk_lexer_get_token(&l);
			while(t.type != TokenCloseBrace) {
				Token *member = 0;
				while(t.type != TokenSemicolon) {
					birk_array_push(member, t);
					birk_lexer_eat_whitespace(&l);
					t = birk_lexer_get_token(&l);
				}
				birk_array_push(child.members, member);
				birk_lexer_eat_whitespace(&l);
				t = birk_lexer_get_token(&l);
			}
			EXPECT(TokenSemicolon);

			birk_array_push(child_structs, child);

			birk_lexer_eat_whitespace(&l);
			t = birk_lexer_get_token(&l);
			continue;
		}

		birk_array_push(rest_of_file, t);
		t = birk_lexer_get_token(&l);
	}

	// Add all orhpanes to parents
	ChildStruct child;
	FOR_ARRAY(child_structs, child) {
		BaseStruct *parent;
		FOR_ARRAY_PTR(base_structs, parent) {
			if(birk_strcmp(child.parent_name, parent->name) == 0) {
				birk_array_push(parent->children, child);
				goto child_loop;
			}
		}

		printf("Child '%s' specified parent '%s', but this parent does not exists!\n", child.name, child.parent_name);
		LEAVE(1);

		child_loop:;
	}
	birk_array_free(child_structs);

	// Predeclarations
	BaseStruct base;
	if(birk_array_count(base_structs) > 0) {
		FOR_ARRAY(base_structs, base) {
			printf("typedef struct %s %s;\n", base.name, base.name);

			ChildStruct child;
			if(birk_array_count(base.children) > 0) {
				FOR_ARRAY(base.children, child) {
					printf("typedef struct %s %s;\n", child.name, child.name);
				}
			}
		}
	}

	printf("\n");

	// Enums
	if(birk_array_count(base_structs) > 0) {
		FOR_ARRAY(base_structs, base) {
			printf("typedef enum %sType %sType;\n", base.name, base.name);
			printf("enum %sType {\n", base.name);

			ChildStruct child;
			if(birk_array_count(base.children) > 0) {
				FOR_ARRAY(base.children, child) {
					printf("\t%s%s,\n", base.name, child.name);
				}
			}

			printf("\t%sNameCount,\n", base.name);

			printf("};\n");
		}
	}

	printf("\n");

	// Names
	if(birk_array_count(base_structs) > 0) {
		FOR_ARRAY(base_structs, base) {
			printf("static const char* %sNames[%sNameCount] = {\n", base.name, base.name);

			ChildStruct child;
			if(birk_array_count(base.children) > 0) {
				FOR_ARRAY(base.children, child) {
					printf("\t\"%s%s\",\n", base.name, child.name);
				}
			}

			printf("};\n");
		}
	}

	printf("\n");

	// Declarations
	if(birk_array_count(base_structs) > 0) {
		FOR_ARRAY(base_structs, base) {
			printf("struct %s {\n", base.name, base.name);

			printf("\t%sType type;\n", base.name);
			Token *tokens;
			FOR_ARRAY(base.members, tokens) {
				Token token;
				printf("\t");
				FOR_ARRAY(tokens, token) {
					printf("%.*s ", token.len, token.text);
				}
				printf(";\n");
			}
			printf("};\n");

			printf("\n");

			ChildStruct child;
			if(birk_array_count(base.children) > 0) {
				FOR_ARRAY(base.children, child) {
					printf("struct %s {\n", child.name);
					printf("\tunion {\n");
					printf("\t\t%s parent;\n", base.name);
					printf("\t\tstruct {\n");
					printf("\t\t\t%sType type;\n", base.name);

					Token *tokens;
					FOR_ARRAY(base.members, tokens) {
						Token token;
						printf("\t\t\t");
						FOR_ARRAY(tokens, token) {
							printf("%.*s ", token.len, token.text);
						}
						printf(";\n");
					}

					// Anon struct end
					printf("\t\t};\n");
					// Union end
					printf("\t};\n");

					FOR_ARRAY(child.members, tokens) {
						Token token;
						printf("\t");
						FOR_ARRAY(tokens, token) {
							printf("%.*s ", token.len, token.text);
						}
						printf(";\n");
					}

					printf("};\n");
					printf("\n");
				}
			}
		}
	}

	printf("\n");

	// Maker functions
	if(birk_array_count(base_structs) > 0) {
		FOR_ARRAY(base_structs, base) {
			ChildStruct child;
			if(birk_array_count(base.children) > 0) {
				FOR_ARRAY(base.children, child) {
					printf("%s make_%s() { %s o = {0}; o.type = %s%s; return o;}\n",
						child.name, child.name, child.name, base.name, child.name);
				}
			}
		}
	}

	printf("\n");

	FOR_ARRAY(rest_of_file, t) {
		printf("%.*s", t.len, t.text);
		//if(t.type == TokenString) printf("\n");
	}

	LEAVE(0);
}