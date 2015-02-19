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

void TranslationUnit(TreeNode<ASTNodeVal>** r_astRootNode, ParserState** r_parserState)
{
	TreeNode<ASTNodeVal>	*astRootNode = new TreeNode<ASTNodeVal>;
	ParserState				*parserState = new ParserState;

	parserState->stack.push_back(Scope());

	astRootNode->val.type = ASTNodeValType::VARIANT;
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
				addChildrenToNode(astRootNode, Declaration(*parserState, true));
			} else {
				astRootNode->addChild(FunctionDefinition(*parserState));
			}
		} else {
			addChildrenToNode(astRootNode, Declaration(*parserState, true));
		}
	}

	parserState->stack.pop_back();

	*r_astRootNode = astRootNode;
	*r_parserState = parserState;
}

TreeNode<ASTNodeVal>* FunctionDefinition(ParserState& parserState)
{
	// open the function's scope
	parserState.stack.push_back(Scope());

	TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;

	Symbol s;

	s.type.spec = Specifier();
	s.type.lvlsOfIndirection = Pointers();
	s.type.isFunction = true;
	s.defined = true;

	Variant v;
	unsigned int lineNumber;
	match("IDENTIFIER", &v, &lineNumber);
	s.identifier = v.getStrVal();

	match("(");
	rootNode->addChild(Parameters(parserState, s.type.funcParams));
	match(")");

	// check if function already defined, if so, error.
	// otherwise define it in the scope.
	ScopeStack::iterator currScope;
	currScope = parserState.stack.begin();

	Scope::const_iterator it = currScope->find(s.identifier);

	SymbolTableRef sym;

	// Is the variable already defined in the current scope?
	// If so, error, otherwise define it.
	if (it != currScope->end()) {
		sym = it->second;

		if (sym->second.defined) {
			outputError(lineNumber, "redefinition of '" + s.identifier + "'");
		} else if (!cmpFuncWithoutParams(s, sym->second)) {
			// don't compare the declaration's function parameters with this
			// definition
			outputError(lineNumber, "conflicting types for '" + s.identifier + "'");
		} else {
			// this is the first definition of the function,
			// so we've already seen its declaration
			sym->second.defined = true;
		}

		// an erroneous function definition always replaces the existing
		// definition, and in the case where this is just the first
		// function's definition we still have to update it to include
		// our function parameters.
		sym->second = s;
	} else {
		sym = parserState.symbolTable.insert(pair<string, Symbol>(s.identifier, s));
		ScopeStack::reverse_iterator i = ++parserState.stack.rbegin();
		(*i)[s.identifier] = sym;
	}

	rootNode->val.symbol = sym;
	rootNode->val.type = ASTNodeValType::SYMBOL;

	Type retType;
	retType.arraySize = sym->second.type.arraySize;
	retType.lvlsOfIndirection = sym->second.type.lvlsOfIndirection;
	retType.spec = sym->second.type.spec;

	match("{");
	// ignore the output of these functions.
	// if they return NULL then they just won't be added
	// to the tree.
	rootNode->addChild(Declarations(parserState));
	rootNode->addChild(Statements(parserState, &retType));
	match("}");

	// close the function's scope
	parserState.stack.pop_back();

	return rootNode;
}

TreeNode<ASTNodeVal>* Parameters(ParserState& parserState, vector<Symbol*>& funcParams)
{
	if (lookahead() == "void") {
		match("void");
		return NULL;
	} else {
		TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;
		rootNode->val.type = ASTNodeValType::VARIANT;
		rootNode->val.variant.setVal(Variant::STRING, "Parameters");

		Symbol* a = new Symbol;

		TreeNode<ASTNodeVal>* paramNode = Parameter(parserState, *a);

		if (paramNode) {
			rootNode->addChild(paramNode);
			funcParams.push_back(a);
		} else {
			delete a;
		}

		while (lookahead() == ",") {
			match(",");

			a = new Symbol;
			paramNode = Parameter(parserState, *a);

			if (paramNode) {
				rootNode->addChild(paramNode);
				funcParams.push_back(a);
			} else {
				delete a;
			}
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

	ScopeStack::reverse_iterator currScope = parserState.stack.rbegin();
	Scope::const_iterator it = currScope->find(s.identifier);

	SymbolTableRef sym;

	// Is the parameter already defined in the current scope?
	// If so, error, otherwise define it.
	if (it != currScope->end()) {
		outputError(lineNumber, "redeclaration of '" + s.identifier + "'");
		return NULL;
	} else {
		sym = parserState.symbolTable.insert(pair<string, Symbol>(s.identifier, s));
		(*currScope)[s.identifier] = sym;
	}

	TreeNode<ASTNodeVal>* node = new TreeNode<ASTNodeVal>;
	node->val.symbol = sym;
	node->val.type = ASTNodeValType::SYMBOL;

	return node;
}

TreeNode<ASTNodeVal>* Declarations(ParserState& parserState)
{
	TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;
	rootNode->val.type = ASTNodeValType::VARIANT;
	rootNode->val.variant.setVal(Variant::STRING, "DECLS");

	for (string currToken = lookahead(); isSpecifier(currToken); currToken = lookahead()) {
		addChildrenToNode(rootNode, Declaration(parserState));
	}

	return rootNode;
}

list<TreeNode<ASTNodeVal>*> Declaration(ParserState& parserState, bool global)
{
	list<TreeNode<ASTNodeVal>*> decls;

	Type::eSpecifier spec = Specifier();

	TreeNode<ASTNodeVal>* declNode = Declarator(parserState, spec, global);

	if (declNode) {
		decls.push_back(declNode);
	}

	while (lookahead() == ",") {
		match(",");

		declNode = Declarator(parserState, spec, global);

		if (declNode) {
			decls.push_back(declNode);
		}
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

	ScopeStack::reverse_iterator currScope;
	currScope = parserState.stack.rbegin();

	Scope::const_iterator it = currScope->find(s.identifier);

	SymbolTableRef newSym;

	// Is the variable already defined in the current scope?
	// If so, error, otherwise define it.
	if (it != currScope->end()) {
		newSym = it->second;

		if (global && !s.type.isFunction && s != newSym->second) {
			outputError(lineNumber, "conflicting types for '" + s.identifier + "'");
			return NULL;
		} else if (global && s.type.isFunction && !cmpFuncWithoutParams(s, newSym->second)) {
			outputError(lineNumber, "conflicting types for '" + s.identifier + "'");
			return NULL;
		} else if (!s.type.isFunction && !global) {
			outputError(lineNumber, "redeclaration of '" + s.identifier + "'");
			return NULL;
		}
	} else {
		newSym = parserState.symbolTable.insert(pair<string, Symbol>(s.identifier, s));
		(*currScope)[s.identifier] = newSym;
	}

	TreeNode<ASTNodeVal>* node = new TreeNode<ASTNodeVal>;
	node->val.symbol = newSym;
	node->val.type = ASTNodeValType::SYMBOL;

	return node;
}

TreeNode<ASTNodeVal>* Statements(ParserState& parserState, Type* enclosingFuncRetType)
{
	TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;
	rootNode->val.type = ASTNodeValType::VARIANT;
	rootNode->val.variant.setVal(Variant::STRING, "STMTS");

	while (lookahead() != "}")
	{
		// ignore if Statement returns NULL or not.
		// equivalent to just skipping over invalid
		// statements.
		rootNode->addChild(Statement(parserState, enclosingFuncRetType));
	}

	return rootNode;
}

TreeNode<ASTNodeVal>* Statement(ParserState& parserState, Type* enclosingFuncRetType)
{
	string currToken = lookahead();

	TreeNode<ASTNodeVal>* astRootNode = new TreeNode<ASTNodeVal>;

	if (currToken == "{") {
		match("{");

		parserState.stack.push_back(Scope());

		astRootNode->val.type = ASTNodeValType::VARIANT;
		astRootNode->val.variant.setVal(Variant::STRING, "BLOCK");

		TreeNode<ASTNodeVal>* declNode = Declarations(parserState);

		if (!declNode) {
			SkipErrorUntil("}");
			match("}");
			parserState.stack.pop_back();
			delete astRootNode;
			return NULL;
		}

		astRootNode->addChild(declNode);

		TreeNode<ASTNodeVal>* stmtsNode = Statements(parserState, enclosingFuncRetType);

		if (!stmtsNode) {
			SkipErrorUntil("}");
			match("}");
			parserState.stack.pop_back();
			delete astRootNode;
			return NULL;
		}

		astRootNode->addChild(stmtsNode);

		parserState.stack.pop_back();

		match("}");
	} else if (currToken == "return") {
		unsigned int lineNumber;
		match("return", NULL, &lineNumber);

		astRootNode->val.type = ASTNodeValType::VARIANTTYPENODE;
		astRootNode->val.variantTypeNode.variant.setVal(Variant::STRING, "RETURN");

		TreeNode<ASTNodeVal>* expNode = Expression(parserState);

		if (!expNode) {
			SkipErrorUntil(";");
			match(";");
			delete astRootNode;
			return NULL;
		} else {
			astRootNode->addChild(expNode);
			match(";");
		}

		Type childType;

		if (!GetSymbolType(expNode->val, &childType)) {
			if (!GetVariantType(expNode->val, &childType)) {
				childType = expNode->val.variantTypeNode.type;
			}
		}

		if (!typesCompatible(childType, *enclosingFuncRetType)) {
			outputError(lineNumber, "invalid return type");
			return NULL;
		}

		astRootNode->val.variantTypeNode.type = childType;
	} else if (currToken == "while") {
		match("while");

		unsigned int lineNumber;

		match("(", NULL, &lineNumber);

		astRootNode->val.type = ASTNodeValType::VARIANT;
		astRootNode->val.variant.setVal(Variant::STRING, "WHILE");

		TreeNode<ASTNodeVal>* conditionNode = Expression(parserState);

		if (!conditionNode) {
			SkipErrorUntil(")");
		} else {
			astRootNode->addChild(conditionNode);
		}

		Type conditionType;

		if (!GetSymbolType(conditionNode->val, &conditionType)) {
			if (!GetVariantType(conditionNode->val, &conditionType)) {
				conditionType = conditionNode->val.variantTypeNode.type;
			}
		}

		if (!isLogicalType(conditionType)) {
			outputError(lineNumber, "invalid type for test expression");
		}

		match(")");

		TreeNode<ASTNodeVal>* stmtNode = Statement(parserState, enclosingFuncRetType);

		if (stmtNode) {
			astRootNode->addChild(stmtNode);
		} else {
			delete astRootNode;
			return NULL;
		}
	} else if (currToken == "if") {
		match("if");

		unsigned int lineNumber;

		match("(", NULL, &lineNumber);

		astRootNode->val.type = ASTNodeValType::VARIANT;
		astRootNode->val.variant.setVal(Variant::STRING, "IF");

		TreeNode<ASTNodeVal>* conditionNode = Expression(parserState);

		if (!conditionNode) {
			SkipErrorUntil(")");
		} else {
			astRootNode->addChild(conditionNode);
		}

		Type conditionType;

		if (!GetSymbolType(conditionNode->val, &conditionType)) {
			if (!GetVariantType(conditionNode->val, &conditionType)) {
				conditionType = conditionNode->val.variantTypeNode.type;
			}
		}

		if (!isLogicalType(conditionType)) {
			outputError(lineNumber, "invalid type for test expression");
		}

		match(")");

		astRootNode->addChild(Statement(parserState, enclosingFuncRetType));

		if (lookahead() == "else") {
			match("else");

			TreeNode<ASTNodeVal>* astElseNode = new TreeNode<ASTNodeVal>(astRootNode);
			astElseNode->val.type = ASTNodeValType::VARIANT;
			astElseNode->val.variant.setVal(Variant::STRING, "ELSE");
			astElseNode->addChild(Statement(parserState, enclosingFuncRetType));
		}
	} else {
		TreeNode<ASTNodeVal>* astExpNode = Expression(parserState);

		if (!astExpNode) {
			SkipErrorUntil(";");
			match(";");
			return NULL;
		}

		if (lookahead() == "=") {
			unsigned int lineNumber;
			match("=", NULL, &lineNumber);

			astRootNode->val.type = ASTNodeValType::VARIANT;
			astRootNode->val.variant.setVal(Variant::STRING, "ASSIGN");
			astRootNode->addChild(astExpNode);

			TreeNode<ASTNodeVal>* valNode = Expression(parserState);

			if (!valNode) {
				SkipErrorUntil(";");
				match(";");
				return NULL;
			}

			match(";");

			astRootNode->addChild(valNode);

			Type t1, t2;

			bool lhsIsLval;

			if (!GetSymbolType(astExpNode->val, &t1, &lhsIsLval)) {
				if (!GetVariantType(astExpNode->val, &t1)) {
					lhsIsLval = astExpNode->val.variantTypeNode.isLvalue;
					t1 = astExpNode->val.variantTypeNode.type;
				} else {
					lhsIsLval = false;
				}
			}

			if (!GetSymbolType(valNode->val, &t2, &lhsIsLval)) {
				if (!GetVariantType(valNode->val, &t2)) {
					t2 = valNode->val.variantTypeNode.type;
				}
			}

			if (!lhsIsLval) {
				outputError(lineNumber, "lvalue required in expression");
				return NULL;
			}

			if (!typesCompatible(t1, t2)) {
				outputError(lineNumber, "invalid operands to binary =");
				return NULL;
			}
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

	TreeNode<ASTNodeVal>* expNode = Expression(parserState);

	if (!expNode) {
		return nodes;
	}

	nodes.push_back(expNode);

	while (lookahead() == ",") {
		match(",");

		expNode = Expression(parserState);

		if (!expNode) {
			FreeNodeList(nodes);
			return nodes;
		}

		nodes.push_back(expNode);
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