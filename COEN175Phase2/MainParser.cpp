#include "Header.h"

void TranslationUnit();
void TranslationUnitPrefix();
void GlobalDeclaratorSuffix();
void FunctionDefinitionSuffix();

void GlobalDeclaration();
void GlobalDeclaratorList();
void GlobalDeclarator();
void Pointers();
void Specifier();
void FunctionDefinition();
void Parameters();
void ParameterList();
void Parameter();
void Declarations();
void Declaration();
void DeclaratorList();
void Declarator();
void Statements();
void Statement();
void ExpressionList();

void Specifier()
{
	if (lookahead() == "int") {
		match("int");
	} else if (lookahead() == "long") {
		match("long");
	} else if (lookahead() == "char") {
		match("char");
	}
}

void Pointers()
{
	while (lookahead() == "*") {
		match("*");
	}
}

void TranslationUnit()
{
	TranslationUnitPrefix();
}

void TranslationUnitPrefix()
{
	Specifier();
	Pointers();
	match("id");
}

void GlobalDeclaratorSuffix()
{
	if (lookahead() == "(")
	{
		match("(");
		match(")");
	}
}