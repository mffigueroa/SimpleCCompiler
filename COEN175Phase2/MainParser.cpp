#include <string>

using namespace std;

#include "Header.h"

ExpressionParser expParser;

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
	if (lookahead(3) == "(") {
		if (lookahead(4) == ")") {
			GlobalDeclaration();
		} else {
			FunctionDefinition();
		}
	} else {
		GlobalDeclaration();
	}
}

void GlobalDeclaration()
{
	Specifier();

	GlobalDeclarator();
	while (lookahead() == ",") {
		match(",");
		GlobalDeclarator();
	}

	match(";");
}

void GlobalDeclarator()
{
	Pointers();
	match("id");
	if (lookahead() == "[")	{
		match("[");
		match("num");
		match("]");
	} else if (lookahead() == "(")	{
		match("(");
		match(")");
	}
}

void FunctionDefinition()
{
	Specifier();
	Pointers();
	match("id");

	match("(");
	Parameters();
	match(")");

	match("{");
	Declarations();
	Statements();
	match("}");
}

void Parameters()
{
	if (lookahead() == "void") {
		match("void");
	} else {
		Parameter();

		while (lookahead() == ",") {
			match(",");
			Parameter();
		}
	}
}

void Parameter()
{
	Specifier();
	Pointers();
	match("id");
}

void Declarations()
{
	for (string currToken = lookahead();
		currToken == "int" || currToken == "long" || currToken == "char";
		currToken = lookahead()) {
			Declaration();
	}
}

void Declaration()
{
	Specifier();
	Declarator();

	while (lookahead() == ",") {
		Declarator();
	}

	match(";");
}

void Declarator()
{
	Pointers();
	match("id");

	if (lookahead() == "[") {
		match("[");
		match("num");
		match("]");
	}
}

void Statements()
{
	while (lookahead() != "}")
	{
		Statement();
	}
}

void Statement()
{
	string currToken = lookahead();

	if (currToken == "{") {
		Declarations();
		Statements();
	} else if (currToken == "return") {
		match("return");
		expParser();
		match(";");
	} else if (currToken == "while") {
		match("while");
		match("(");
		expParser();
		match(")");
		Statement();
	} 
	// --- WHAT ABOUT IF-ELSE ---
	else if (currToken == "if") {
		match("if");
		match("(");
		expParser();
		match(")");
		Statement();
	} else {
		expParser();
		if (lookahead() == "=") {
			match("=");
			expParser();
			match(";");
		} else {
			match(";");
		}
	}
}

void ExpressionList()
{
	expParser();

	while (lookahead() == ",") {
		expParser();
	}
}