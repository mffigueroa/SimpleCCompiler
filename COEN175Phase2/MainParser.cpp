#include <string>
#include <vector>

using namespace std;

#include "Header.h"
#include "Type.h"
#include "Symbol.h"
#include "Tree.h"

eSpecifier Specifier()
{
	if (lookahead() == "int") {
		match("int");
		return INT;
	} else if (lookahead() == "long") {
		match("long");
		return LONGINT;
	} else if (lookahead() == "char") {
		match("char");
		return CHAR;
	}

	return UNDEFINED;
}

size_t Pointers()
{
	int indirectionLvls = 0;
	while (lookahead() == "*") {
		++indirectionLvls;
		match("*");
	}

	return indirectionLvls;
}

void TranslationUnit()
{
	TreeNode<vector<Symbol>> transUnitScope;

	while (lookahead() != "EOF") {
		// both GlobalDeclaration and FunctionDefinition
		// start with Specifier Pointers ID

		// starting at the first possible position
		// for Pointers, find the end of Pointers
		int lastPointerToken = 1;
		for (; lookahead(lastPointerToken) == "*"; ++lastPointerToken);

		if (lookahead(lastPointerToken + 1) == "(") {
			if (lookahead(lastPointerToken + 2) == ")") {
				GlobalDeclaration(transUnitScope);
			} else {
				FunctionDefinition(transUnitScope);
			}
		} else {
			GlobalDeclaration(transUnitScope);
		}
	}
}

void GlobalDeclaration(TreeNode<vector<Symbol>>& scope)
{
	eSpecifier spec = Specifier();

	Symbol s;
	GlobalDeclarator(s, spec);
	scope.getVal().push_back(s);

	while (lookahead() == ",") {
		match(",");
		GlobalDeclarator(s, spec);
		scope.getVal().push_back(s);
	}

	match(";");
}

void GlobalDeclarator(Symbol& s, eSpecifier spec)
{
	s.type.lvlsOfIndirection = Pointers();

	Variant ident;
	match("IDENTIFIER", ident);

	s.identifier = ident.getStrVal();
	s.type.spec = spec;
	s.type.isFunction = false;
	s.type.arraySize = 0;

	if (lookahead() == "[")	{
		match("[");
		s.type.arraySize = Number();
		match("]");
	} else if (lookahead() == "(")	{
		match("(");
		match(")");

		s.type.isFunction = true;
	}
}

void FunctionDefinition(TreeNode<vector<Symbol>>& scope)
{
	TreeNode<vector<Symbol>>* funcScope = new TreeNode<vector<Symbol>>(scope);

	Symbol s;

	s.type.spec = Specifier();
	s.type.lvlsOfIndirection = Pointers();
	s.type.isFunction = true;

	Variant v;
	match("IDENTIFIER", v);
	s.identifier = v.getStrVal();

	match("(");
	Parameters(s.type.funcParams);
	match(")");

	match("{");
	Declarations(*funcScope);
	Statements(*funcScope);
	match("}");
}

void Parameters(vector<Symbol*>& params)
{
	if (lookahead() == "void") {
		match("void");
	} else {
		Symbol* a = new Symbol;
		Parameter(*a);
		params.push_back(a);

		while (lookahead() == ",") {
			match(",");

			a = new Symbol;
			Parameter(*a);
			params.push_back(a);
		}
	}
}

void Parameter(Symbol& s)
{
	s.type.spec = Specifier();
	s.type.lvlsOfIndirection = Pointers();

	Variant v;
	match("IDENTIFIER", v);
	s.identifier = v.getStrVal();
}

void Declarations(TreeNode<vector<Symbol>>& scope)
{
	for (string currToken = lookahead();
		currToken == "int" || currToken == "long" || currToken == "char";
		currToken = lookahead()) {
			Declaration(scope);
	}
}

void Declaration(TreeNode<vector<Symbol>>& scope)
{
	eSpecifier spec = Specifier();

	Symbol a;
	Declarator(a, spec);
	scope.getVal().push_back(a);

	while (lookahead() == ",") {
		match(",");

		Symbol b;
		Declarator(b, spec);
		scope.getVal().push_back(b);
	}

	match(";");
}

void Declarator(Symbol& s, eSpecifier spec)
{
	s.type.lvlsOfIndirection = Pointers();

	Variant ident;
	match("IDENTIFIER", ident);

	s.identifier = ident.getStrVal();
	s.type.spec = spec;
	s.type.isFunction = false;
	s.type.arraySize = 0;

	if (lookahead() == "[")	{
		match("[");
		s.type.arraySize = Number();
		match("]");
	}
}

void Statements(TreeNode<vector<Symbol>>& scope)
{
	while (lookahead() != "}")
	{
		Statement(scope);
	}
}

void Statement(TreeNode<vector<Symbol>>& scope)
{
	string currToken = lookahead();

	if (currToken == "{") {
		match("{");

		TreeNode<vector<Symbol>>* blockScope = new TreeNode<vector<Symbol>>(&scope);

		Declarations(*blockScope);
		Statements(*blockScope);

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
		Statement(scope);
	} else if (currToken == "if") {
		match("if");
		match("(");
		Expression();
		match(")");
		Statement(scope);

		if (lookahead() == "else") {
			match("else");
			Statement(scope);
		}
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

int Number()
{
	Variant v;

	if (lookahead() == "INTEGER") {
		match("INTEGER", v);
	} else {
		match("LONGINTEGER", v);
	}

	return v.getIntVal();
}