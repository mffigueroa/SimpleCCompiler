%top{
#include <iostream>
#include <string>
#include <map>
#include "Tokens.h"
#include "Type.h"
#include "Variant.h"
#include "Header.h"

using namespace std;

Variant				currVariant;
unsigned int		currLineNumber = 1;

void updateLineNumber(const char* sz);
}

DIGIT	[0-9]
LETTER	[a-zA-Z]
IDENTIFIER	[a-zA-Z_][a-zA-Z0-9_]*
KEYWORD	auto|break|case|char|const|continue|default|do|double|else|enum|extern|float|for|goto|if|int|long|register|return|short|signed|sizeof|static|struct|switch|typedef|union|unsigned|void|volatile|while
INTEGER	0|[1-9][0-9]*
LONGINTEGER	(0|[1-9][0-9]*)[Ll]
OPERATOR	=|\+\+|--|\|\||&&|==|!=|<|>|<=|>=|\+|-|\*|\/|%|&|!|->|\(|\)|\[|\]|\{|\}|;|:|\.|,
STRING	\"([^\\\"\n]*(\\.)?)*\"
CHARACTER	\'([^\\\'\n]*(\\.)?)*\'
WHITESPACE	[\n\t\v\r ]+

%x INCOMMENT

%option noyywrap nounput

%%


<INCOMMENT>\*\/ { BEGIN(INITIAL); }
<INCOMMENT>[^\n] {}
<INCOMMENT>{WHITESPACE} { updateLineNumber(yytext); }
\/\* { BEGIN(INCOMMENT); }
{KEYWORD} { return tokenMap[yytext]; }
{IDENTIFIER} { currVariant.setVal(Variant::IDENTIFIER, yytext); return tokenMap["IDENTIFIER"]; }
{STRING} { updateLineNumber(yytext); currVariant.setVal(Variant::STRING, yytext); return tokenMap["STRING"]; }
{CHARACTER} { updateLineNumber(yytext); currVariant.setVal(Variant::CHAR, yytext); return tokenMap["CHARACTER"]; }
{OPERATOR} { return tokenMap[yytext]; }
{INTEGER} { currVariant.setVal(Variant::INT, yytext); return tokenMap["INTEGER"]; }
{LONGINTEGER} { currVariant.setVal(Variant::LONGINT, yytext); return tokenMap["LONGINTEGER"]; }
{WHITESPACE} { updateLineNumber(yytext); }
. { cerr << "Invalid character: " << yytext << endl; }

%%

void updateLineNumber(const char* sz)
{
	for(; *sz; ++sz) {
		if(*sz == '\n') {
			++currLineNumber;
		}
	}
}