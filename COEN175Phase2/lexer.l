%top{
#include <iostream>
#include <string>
#include <map>
#include "Tokens.h"
#include "Type.h"
#include "Variant.h"

using namespace std;

Variant currVariant;
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
<INCOMMENT>(.|\n) {}
<INCOMMENT>{WHITESPACE} {}
\/\* { BEGIN(INCOMMENT); }
{KEYWORD} { return tokenMap[yytext]; }
{IDENTIFIER} { currVariant.setVal(IDENTIFIER, yytext); return tokenMap["IDENTIFIER"]; }
{STRING} { currVariant.setVal(STRING, yytext); return tokenMap["STRING"]; }
{CHARACTER} { currVariant.setVal(CHAR, yytext); return tokenMap["CHARACTER"]; }
{OPERATOR} { return tokenMap[yytext]; }
{INTEGER} { currVariant.setVal(INT, yytext); return tokenMap["INTEGER"]; }
{LONGINTEGER} { currVariant.setVal(LONGINT, yytext); return tokenMap["LONGINTEGER"]; }
{WHITESPACE} {}
. { cerr << "Invalid character: " << yytext << endl; }

%%
