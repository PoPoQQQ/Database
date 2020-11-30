#include <iostream>
#include <cstring>
#include <stdio.h>
using namespace std;

extern FILE *yyin;
extern int yyparse();

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s filename\n", argv[0]);
        return 0;
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        printf("Open file failed: %s\n", argv[1]);
        return 0;
    }

	printf("-----begin parsing %s\n", argv[1]);
	yyparse();						//使yacc开始读取输入和解析，它会调用lex的yylex()读取记号
	puts("-----end parsing");

	fclose(yyin);

    yyin = NULL;

	return 0;
}