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

TreeNode<Variant>* TranslationUnit()
{
	TreeNode<Variant>*	astRootNode = new TreeNode<Variant>;
	ScopeStack			variableScope, functionScope;

	variableScope.push_back(Scope());
	functionScope.push_back(Scope());

	astRootNode->getVal().setVal(STRING, "TranslationUnit");

	while (lookahead() != "EOF") {
		// both GlobalDeclaration and FunctionDefinition
		// start with Specifier Pointers ID

		// starting at the first possible position
		// for Pointers, find the end of Pointers
		int lastPointerToken = 1;
		for (; lookahead(lastPointerToken) == "*"; ++lastPointerToken);

		if (lookahead(lastPointerToken + 1) == "(") {
			if (lookahead(lastPointerToken + 2) == ")") {
				astRootNode->addChild(GlobalDeclaration(variableScope, functionScope));
			} else {
				astRootNode->addChild(FunctionDefinition(variableScope, functionScope));
			}
		} else {
			astRootNode->addChild(GlobalDeclaration(variableScope, functionScope));
		}
	}

	variableScope.pop_back();
	functionScope.pop_back();

	return astRootNode;
}

TreeNode<Variant>* GlobalDeclaration(ScopeStack& variableStack, ScopeStack& functionStack)
{
	TreeNode<Variant>* rootNode = new TreeNode<Variant>;
	eSpecifier spec = Specifier();

	rootNode->getVal().setVal(STRING, GetSpecifierName(spec));
	rootNode->addChild(GlobalDeclarator(variableStack, functionStack, spec));

	while (lookahead() == ",") {
		match(",");
		rootNode->addChild(GlobalDeclarator(variableStack, functionStack, spec));
	}

	match(";");
	return rootNode;
}

TreeNode<Variant>* GlobalDeclarator(ScopeStack& variableStack, ScopeStack& functionStack, eSpecifier spec)
{
	Symbol s;
	s.type.lvlsOfIndirection = Pointers();

	Variant ident;
	unsigned int lineNumber;
	match("IDENTIFIER", &ident, &lineNumber);

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

	// if the variable has been declared already, make sure
	// its of the same type. if not, declare it in the scope.
	Symbol symbolLookup;
	if (LookupSymbol(variableStack, s.identifier, &symbolLookup)) {
		if (s != symbolLookup) {
			outputError(lineNumber, "conflicting types for '" + s.identifier + "'");
		}
	} else {
		variableStack.back()[s.identifier] = s;
	}

	TreeNode<Variant>* node = new TreeNode<Variant>;
	node->getVal().setVal(STRING, ident.getStrVal());

	return node;
}

TreeNode<Variant>* FunctionDefinition(ScopeStack& variableStack, ScopeStack& functionStack)
{
	// open the function's scope
	variableStack.push_back(Scope());
	functionStack.push_back(Scope());

	TreeNode<Variant>* rootNode = new TreeNode<Variant>;

	Symbol s;

	s.type.spec = Specifier();
	s.type.lvlsOfIndirection = Pointers();
	s.type.isFunction = true;

	Variant v;
	unsigned int lineNumber;
	match("IDENTIFIER", &v, &lineNumber);
	s.identifier = v.getStrVal();

	rootNode->getVal().setVal(STRING, s.identifier);

	match("(");
	rootNode->addChild(Parameters(variableStack, functionStack, s.type.funcParams));
	match(")");

	// check if function already defined, if so, error.
	// otherwise define it in the scope.
	Symbol lookupSymbol;
	if (LookupSymbol(functionStack, s.identifier, &lookupSymbol)) {
		outputError(lineNumber, "redefinition of '" + s.identifier + "'");
	} else {
		ScopeStack::reverse_iterator i = ++functionStack.rbegin();
		(*i)[s.identifier] = s;
	}

	match("{");
	rootNode->addChild(Declarations(variableStack, functionStack));
	rootNode->addChild(Statements(variableStack, functionStack));
	match("}");

	// close the function's scope
	functionStack.pop_back();
	variableStack.pop_back();

	return rootNode;
}

TreeNode<Variant>* Parameters(ScopeStack& variableStack, ScopeStack& functionStack, vector<Symbol*>& funcParams)
{
	if (lookahead() == "void") {
		match("void");
		return NULL;
	} else {
		TreeNode<Variant>* rootNode = new TreeNode<Variant>;
		rootNode->getVal().setVal(STRING, "Parameters");

		Symbol* a = new Symbol;
		rootNode->addChild(Parameter(variableStack, functionStack, *a));
		funcParams.push_back(a);

		while (lookahead() == ",") {
			match(",");

			a = new Symbol;
			rootNode->addChild(Parameter(variableStack, functionStack, *a));
			funcParams.push_back(a);
		}

		return rootNode;
	}
}

TreeNode<Variant>* Parameter(ScopeStack& variableStack, ScopeStack& functionStack, Symbol& s)
{
	s.type.spec = Specifier();
	s.type.lvlsOfIndirection = Pointers();

	Variant v;
	unsigned int lineNumber;
	match("IDENTIFIER", &v, &lineNumber);
	s.identifier = v.getStrVal();

	ScopeStack::reverse_iterator currScope = variableStack.rbegin();
	Scope::const_iterator it = currScope->find(s.identifier);
	// Is the parameter already defined in the current scope?
	// If so, error, otherwise define it.
	if (it != currScope->end()) {
		outputError(lineNumber, "redeclaration of '" + s.identifier + "'");
	} else {
		(*currScope)[s.identifier] = s;
	}

	TreeNode<Variant>* node = new TreeNode<Variant>;
	node->getVal().setVal(STRING, GetSpecifierName(s.type.spec) + "_" + s.identifier);
	return node;
}

TreeNode<Variant>* Declarations(ScopeStack& variableStack, ScopeStack& functionStack)
{
	TreeNode<Variant>* rootNode = new TreeNode<Variant>;
	rootNode->getVal().setVal(STRING, "Decls");

	for (string currToken = lookahead();
		currToken == "int" || currToken == "long" || currToken == "char";
		currToken = lookahead()) {
			list<TreeNode<Variant>*> nodes = Declaration(variableStack, functionStack);
			
			for (list<TreeNode<Variant>*>::const_iterator it = nodes.begin(), it_end = nodes.end();
				it != it_end; ++it) {
					rootNode->addChild(*it);
			}
	}

	return rootNode;
}

list<TreeNode<Variant>*> Declaration(ScopeStack& variableStack, ScopeStack& functionStack)
{
	list<TreeNode<Variant>*> decls;

	eSpecifier spec = Specifier();

	decls.push_back(Declarator(variableStack, functionStack, spec));

	while (lookahead() == ",") {
		match(",");

		decls.push_back(Declarator(variableStack, functionStack, spec));
	}

	match(";");
	return decls;
}

TreeNode<Variant>* Declarator(ScopeStack& variableStack, ScopeStack& functionStack, eSpecifier spec)
{
	Symbol s;
	s.type.lvlsOfIndirection = Pointers();

	Variant ident;
	unsigned int lineNumber;
	match("IDENTIFIER", &ident, &lineNumber);

	s.identifier = ident.getStrVal();
	s.type.spec = spec;
	s.type.isFunction = false;
	s.type.arraySize = 0;

	TreeNode<Variant>* node = new TreeNode<Variant>;
	node->getVal().setVal(STRING, GetSpecifierName(spec) + "_" + s.identifier);

	if (lookahead() == "[")	{
		match("[");
		s.type.arraySize = Number();
		match("]");
	}

	ScopeStack::reverse_iterator currScope = variableStack.rbegin();
	Scope::const_iterator it = currScope->find(s.identifier);
	// Is the variable already defined in the current scope?
	// If so, error, otherwise define it.
	if (it != currScope->end()) {
		outputError(lineNumber, "redeclaration of '" + s.identifier + "'");
	}
	else {
		(*currScope)[s.identifier] = s;
	}

	return node;
}

TreeNode<Variant>* Statements(ScopeStack& variableStack, ScopeStack& functionStack)
{
	TreeNode<Variant>* rootNode = new TreeNode<Variant>;
	rootNode->getVal().setVal(STRING, "stmts");

	while (lookahead() != "}")
	{
		rootNode->addChild(Statement(variableStack, functionStack));
	}

	return rootNode;
}

TreeNode<Variant>* Statement(ScopeStack& variableStack, ScopeStack& functionStack)
{
	string currToken = lookahead();

	TreeNode<Variant>* astRootNode = new TreeNode<Variant>;

	if (currToken == "{") {
		match("{");

		variableStack.push_back(Scope());

		astRootNode->getVal().setVal(STRING, "block");

		astRootNode->addChild(Declarations(variableStack, functionStack));
		astRootNode->addChild(Statements(variableStack, functionStack));

		variableStack.pop_back();

		match("}");
	} else if (currToken == "return") {
		match("return");

		astRootNode->getVal().setVal(STRING, "return");

		astRootNode->addChild(Expression(variableStack, functionStack));
		match(";");
	} else if (currToken == "while") {
		match("while");
		match("(");

		astRootNode->getVal().setVal(STRING, "while");

		astRootNode->addChild(Expression(variableStack, functionStack));
		match(")");
		astRootNode->addChild(Statement(variableStack, functionStack));
	} else if (currToken == "if") {
		match("if");
		match("(");

		astRootNode->getVal().setVal(STRING, "if");

		astRootNode->addChild(Expression(variableStack, functionStack));
		match(")");

		astRootNode->addChild(Statement(variableStack, functionStack));

		if (lookahead() == "else") {
			match("else");

			TreeNode<Variant>* astElseNode = new TreeNode<Variant>(astRootNode);
			astElseNode->getVal().setVal(STRING, "else");
			astElseNode->addChild(Statement(variableStack, functionStack));
		}
	} else {
		TreeNode<Variant>* astExpNode = Expression(variableStack, functionStack);

		if (lookahead() == "=") {
			match("=");

			astRootNode->getVal().setVal(STRING, "assign");
			astRootNode->addChild(astExpNode);
			astRootNode->addChild(Expression(variableStack, functionStack));
			match(";");
		} else {
			match(";");
			return astExpNode;
		}
	}

	return astRootNode;
}

list<TreeNode<Variant>*> ExpressionList(ScopeStack& variableStack, ScopeStack& functionStack)
{
	list<TreeNode<Variant>*> nodes;
	nodes.push_back(Expression(variableStack, functionStack));

	while (lookahead() == ",") {
		match(",");
		nodes.push_back(Expression(variableStack, functionStack));
	}

	return nodes;
}

int Number()
{
	Variant v;

	if (lookahead() == "INTEGER") {
		match("INTEGER", &v);
	} else {
		match("LONGINTEGER", &v);
	}

	return v.getIntVal();
}