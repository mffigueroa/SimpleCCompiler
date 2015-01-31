#include <string>
#include <vector>

using namespace std;

#include "Header.h"
#include "Type.h"
#include "Symbol.h"
#include "Tree.h"

Type::eSpecifier Specifier()
{
	if (lookahead() == "int") {
		match("int");
		return Type::INT;
	} else if (lookahead() == "long") {
		match("long");
		return Type::LONGINT;
	} else {
		match("char");
		return Type::CHAR;
	}
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

TreeNode<ASTNodeVal>* TranslationUnit()
{
	TreeNode<ASTNodeVal>*	astRootNode = new TreeNode<ASTNodeVal>;
	ParserState				parserState;

	parserState.variableStack.push_back(Scope());
	parserState.functionStack.push_back(Scope());

	astRootNode->val.variant.setVal(Variant::STRING, "TranslationUnit");

	while (lookahead() != "EOF") {
		// both GlobalDeclaration and FunctionDefinition
		// start with Specifier Pointers ID

		// starting at the first possible position
		// for Pointers, find the end of Pointers
		int lastPointerToken = 1;
		for (; lookahead(lastPointerToken) == "*"; ++lastPointerToken);

		if (lookahead(lastPointerToken + 1) == "(") {
			if (lookahead(lastPointerToken + 2) == ")") {
				astRootNode->addChild(GlobalDeclaration(parserState));
			} else {
				astRootNode->addChild(FunctionDefinition(parserState));
			}
		} else {
			astRootNode->addChild(GlobalDeclaration(parserState));
		}
	}

	parserState.variableStack.pop_back();
	parserState.functionStack.pop_back();

	return astRootNode;
}

TreeNode<ASTNodeVal>* GlobalDeclaration(ParserState& parserState)
{
	TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;
	Type::eSpecifier spec = Specifier();

	rootNode->val.variant.setVal(Variant::STRING, "DECL");
	rootNode->addChild(GlobalDeclarator(parserState, spec));

	while (lookahead() == ",") {
		match(",");
		rootNode->addChild(GlobalDeclarator(parserState, spec));
	}

	match(";");
	return rootNode;
}

TreeNode<ASTNodeVal>* GlobalDeclarator(ParserState& parserState, Type::eSpecifier spec)
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
	SymbolTableRef symbolLookup;
	if (LookupSymbol(parserState.variableStack, s.identifier, &symbolLookup)) {
		if (s != symbolLookup->second) {
			outputError(lineNumber, "conflicting types for '" + s.identifier + "'");
		}
	} else {
		SymbolTableRef it = parserState.symbolTable.insert(pair<string, Symbol>(s.identifier, s));
		parserState.variableStack.back()[s.identifier] = it;
	}

	TreeNode<ASTNodeVal>* node = new TreeNode<ASTNodeVal>;
	node->val.variant.setVal(Variant::STRING, ident.getStrVal());

	return node;
}

TreeNode<ASTNodeVal>* FunctionDefinition(ParserState& parserState)
{
	// open the function's scope
	parserState.variableStack.push_back(Scope());
	parserState.functionStack.push_back(Scope());

	TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;

	Symbol s;

	s.type.spec = Specifier();
	s.type.lvlsOfIndirection = Pointers();
	s.type.isFunction = true;

	Variant v;
	unsigned int lineNumber;
	match("IDENTIFIER", &v, &lineNumber);
	s.identifier = v.getStrVal();

	match("(");
	rootNode->addChild(Parameters(parserState, s.type.funcParams));
	match(")");

	// check if function already defined, if so, error.
	// otherwise define it in the scope.
	SymbolTableRef sym;
	if (LookupSymbol(parserState.functionStack, s.identifier, &sym)) {
		outputError(lineNumber, "redefinition of '" + s.identifier + "'");
	} else {
		sym = parserState.symbolTable.insert(pair<string, Symbol>(s.identifier, s));
		ScopeStack::reverse_iterator i = ++parserState.functionStack.rbegin();
		(*i)[s.identifier] = sym;
	}

	rootNode->val.symbol = sym;
	rootNode->val.isSymbol = true;

	match("{");
	rootNode->addChild(Declarations(parserState));
	rootNode->addChild(Statements(parserState));
	match("}");

	// close the function's scope
	parserState.functionStack.pop_back();
	parserState.variableStack.pop_back();

	return rootNode;
}

TreeNode<ASTNodeVal>* Parameters(ParserState& parserState, vector<Symbol*>& funcParams)
{
	if (lookahead() == "void") {
		match("void");
		return NULL;
	} else {
		TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;
		rootNode->val.variant.setVal(Variant::STRING, "Parameters");

		Symbol* a = new Symbol;
		rootNode->addChild(Parameter(parserState, *a));
		funcParams.push_back(a);

		while (lookahead() == ",") {
			match(",");

			a = new Symbol;
			rootNode->addChild(Parameter(parserState, *a));
			funcParams.push_back(a);
		}

		return rootNode;
	}
}

TreeNode<ASTNodeVal>* Parameter(ParserState& parserState, Symbol& s)
{
	s.type.spec = Specifier();
	s.type.lvlsOfIndirection = Pointers();

	Variant v;
	unsigned int lineNumber;
	match("IDENTIFIER", &v, &lineNumber);
	s.identifier = v.getStrVal();

	ScopeStack::reverse_iterator currScope = parserState.variableStack.rbegin();
	Scope::const_iterator it = currScope->find(s.identifier);

	SymbolTableRef sym;

	// Is the parameter already defined in the current scope?
	// If so, error, otherwise define it.
	if (it != currScope->end()) {
		outputError(lineNumber, "redeclaration of '" + s.identifier + "'");
	} else {
		sym = parserState.symbolTable.insert(pair<string, Symbol>(s.identifier, s));
		(*currScope)[s.identifier] = sym;
	}

	TreeNode<ASTNodeVal>* node = new TreeNode<ASTNodeVal>;
	node->val.symbol = sym;
	node->val.isSymbol = true;

	return node;
}

TreeNode<ASTNodeVal>* Declarations(ParserState& parserState)
{
	TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;
	rootNode->val.variant.setVal(Variant::STRING, "DECLS");

	for (string currToken = lookahead(); isSpecifier(currToken); currToken = lookahead()) {
			list<TreeNode<ASTNodeVal>*> nodes = Declaration(parserState);
			
			for (list<TreeNode<ASTNodeVal>*>::const_iterator it = nodes.begin(), it_end = nodes.end();
				it != it_end; ++it) {
					rootNode->addChild(*it);
			}
	}

	return rootNode;
}

list<TreeNode<ASTNodeVal>*> Declaration(ParserState& parserState, bool global)
{
	list<TreeNode<ASTNodeVal>*> decls;

	Type::eSpecifier spec = Specifier();

	decls.push_back(Declarator(parserState, spec, global));

	while (lookahead() == ",") {
		match(",");

		decls.push_back(Declarator(parserState, spec, global));
	}

	match(";");
	return decls;
}

TreeNode<ASTNodeVal>* Declarator(ParserState& parserState, Type::eSpecifier spec, bool global)
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
	} else if (global && lookahead() == "(") {
		match("(");
		match(")");

		s.type.isFunction = true;
	}

	ScopeStack::reverse_iterator currScope = parserState.variableStack.rbegin();
	Scope::const_iterator it = currScope->find(s.identifier);

	SymbolTableRef newSym;

	// Is the variable already defined in the current scope?
	// If so, error, otherwise define it.
	if (it != currScope->end()) {
		newSym = it->second;

		if (global && s != newSym->second) {
			outputError(lineNumber, "conflicting types for '" + s.identifier + "'");
		} else {
			outputError(lineNumber, "redeclaration of '" + s.identifier + "'");
		}
	} else {
		newSym = parserState.symbolTable.insert(pair<string, Symbol>(s.identifier, s));
		(*currScope)[s.identifier] = newSym;
	}

	TreeNode<ASTNodeVal>* node = new TreeNode<ASTNodeVal>;
	node->val.symbol = newSym;
	node->val.isSymbol = true;

	return node;
}

TreeNode<ASTNodeVal>* Statements(ParserState& parserState)
{
	TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;
	rootNode->val.variant.setVal(Variant::STRING, "STMTS");

	while (lookahead() != "}")
	{
		rootNode->addChild(Statement(parserState));
	}

	return rootNode;
}

TreeNode<ASTNodeVal>* Statement(ParserState& parserState)
{
	string currToken = lookahead();

	TreeNode<ASTNodeVal>* astRootNode = new TreeNode<ASTNodeVal>;

	if (currToken == "{") {
		match("{");

		parserState.variableStack.push_back(Scope());

		astRootNode->val.variant.setVal(Variant::STRING, "BLOCK");

		astRootNode->addChild(Declarations(parserState));
		astRootNode->addChild(Statements(parserState));

		parserState.variableStack.pop_back();

		match("}");
	} else if (currToken == "return") {
		match("return");

		astRootNode->val.variant.setVal(Variant::STRING, "RETURN");

		astRootNode->addChild(Expression(parserState));
		match(";");
	} else if (currToken == "while") {
		match("while");
		match("(");

		astRootNode->val.variant.setVal(Variant::STRING, "WHILE");

		astRootNode->addChild(Expression(parserState));
		match(")");
		astRootNode->addChild(Statement(parserState));
	} else if (currToken == "if") {
		match("if");
		match("(");

		astRootNode->val.variant.setVal(Variant::STRING, "IF");

		astRootNode->addChild(Expression(parserState));
		match(")");

		astRootNode->addChild(Statement(parserState));

		if (lookahead() == "else") {
			match("else");

			TreeNode<ASTNodeVal>* astElseNode = new TreeNode<ASTNodeVal>(astRootNode);
			astElseNode->val.variant.setVal(Variant::STRING, "ELSE");
			astElseNode->addChild(Statement(parserState));
		}
	} else {
		TreeNode<ASTNodeVal>* astExpNode = Expression(parserState);

		if (lookahead() == "=") {
			match("=");

			astRootNode->val.variant.setVal(Variant::STRING, "ASSIGN");
			astRootNode->addChild(astExpNode);
			astRootNode->addChild(Expression(parserState));
			match(";");
		} else {
			match(";");
			return astExpNode;
		}
	}

	return astRootNode;
}

list<TreeNode<ASTNodeVal>*> ExpressionList(ParserState& parserState)
{
	list<TreeNode<ASTNodeVal>*> nodes;
	nodes.push_back(Expression(parserState));

	while (lookahead() == ",") {
		match(",");
		nodes.push_back(Expression(parserState));
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