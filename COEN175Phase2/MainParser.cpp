#include <string>

using namespace std;

#include "Header.h"

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
	while (lookahead() != "EOF") {
		// both GlobalDeclaration and FunctionDefinition
		// start with Specifier Pointers ID

		// starting at the first possible position
		// for Pointers, find the end of Pointers
		int lastPointerToken = 1;
		for (; lookahead(lastPointerToken) == "*"; ++lastPointerToken);

		if (lookahead(lastPointerToken + 1) == "(") {
			if (lookahead(lastPointerToken + 2) == ")") {
				GlobalDeclaration();
			} else {
				FunctionDefinition();
			}
		} else {
			GlobalDeclaration();
		}
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
	match("IDENTIFIER");
	if (lookahead() == "[")	{
		match("[");
		Number();
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
	match("IDENTIFIER");

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
	match("IDENTIFIER");
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
		match(",");
		Declarator();
	}

	match(";");
}

void Declarator()
{
	Pointers();
	match("IDENTIFIER");

	if (lookahead() == "[") {
		match("[");
		Number();
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
		match("{");
		Declarations();
		Statements();
		match("}");
	} else if (currToken == "return") {
		match("return");
		Expression();
		match(";");
	} else if (currToken == "while") {
		match("while");
		match("(");
		Expression();
		match(")");
		Statement();
	} 
	// --- WHAT ABOUT IF-ELSE ---
	else if (currToken == "if") {
		match("if");
		match("(");
		Expression();
		match(")");
		Statement();
	} else {
		Expression();
		if (lookahead() == "=") {
			match("=");
			Expression();
			match(";");
		} else {
			match(";");
		}
	}
}

void ExpressionList()
{
	Expression();

	while (lookahead() == ",") {
		match(",");
		Expression();
	}
}

void Number()
{
	if (lookahead() == "INTEGER") {
		match("INTEGER");
	} else {
		match("LONGINTEGER");
	}
}