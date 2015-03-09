#include "Header.h"
#include "Tokens.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <deque>
#include <string>

extern int	yylex (void);

extern Variant			currVariant;
extern unsigned int		currLineNumber;

using namespace std;

typedef pair<Variant, unsigned int>		TokenDouble;
typedef pair<int, TokenDouble>			TokenTriple;

deque<TokenTriple>	tokenBuffer;
size_t				currTokenIndex;

void error(unsigned int lineNumber)
{
	cerr << "Error on line " << lineNumber << "." << endl;
	exit(0);
}

void match(const string& tokenType, Variant* v, unsigned int* lineNumber)
{
	TokenTriple currToken;

	if (tokenBuffer.size()) {
		currToken = tokenBuffer.front();
		tokenBuffer.pop_front();
	} else {
		currToken.first = yylex();
		currToken.second.first = currVariant;
		currToken.second.second = currLineNumber;
	}

	if (currToken.first != tokenMap[tokenType]) {
		error(currToken.second.second);
	}

	if (v) {
		*v = currToken.second.first;
	}
	if (lineNumber) {
		*lineNumber = currToken.second.second;
	}
}

string lookahead(unsigned int ahead)
{
	if(ahead + 1 > tokenBuffer.size()) {
	  int tokensToRead = ahead - tokenBuffer.size() + 1;
	  
	  for(int i = 0; i < tokensToRead; ++i) {
		  int tok = yylex();
		  TokenTriple currToken(tok, TokenDouble(currVariant, currLineNumber));
		  tokenBuffer.push_back(currToken);
	  }
	}

	deque<TokenTriple>::const_iterator it = tokenBuffer.begin() + ahead;
	return reverseTokenMap[it->first];
}

bool LookupSymbol(const ScopeStack& stack, const std::string& symbolName, SymbolTableRef* r_symbol)
{
	for (ScopeStack::const_iterator i = stack.begin(); i != stack.end(); ++i) {
		Scope::const_iterator j = i->find(symbolName);
		if (j != i->end()) {
			if (r_symbol) {
				*r_symbol = j->second;
			}

			return true;
		}
	}

	return false;
}

void SkipErrorUntil(const string& synchronizeToken)
{
	for (string la = lookahead(); la != synchronizeToken; la = lookahead()) {
		match(la);
	}
}

void FreeNodeList(list<TreeNode<ASTNodeVal>*>& children)
{
	list<TreeNode<ASTNodeVal>*>::iterator it = children.begin(), it_end = children.end();

	for (; it != it_end; ++it) {
		if (*it) {
			delete *it;
			*it = 0;
		}
	}

	children.clear();
}

bool addChildrenToNode(TreeNode<ASTNodeVal>* v, const list<TreeNode<ASTNodeVal>*>& children)
{
	if (children.empty()) {
		return false;
	}

	list<TreeNode<ASTNodeVal>*>::const_iterator it = children.begin(), it_end = children.end();

	for (; it != it_end; ++it) {
		v->addChild(*it);
	}

	return true;
}

void outputError(unsigned int lineNumber, const std::string& err)
{
	cerr << "line " << lineNumber << ": " << err << endl;
}

bool isSpecifier(const std::string& str)
{
	return str == "int" || str == "long" || str == "char";
}

bool cmpFuncWithoutParams(const Symbol& lhs, const Symbol& rhs)
{
	return	lhs.identifier == rhs.identifier &&
		lhs.type.spec == rhs.type.spec &&
		lhs.type.arraySize == rhs.type.arraySize &&
		lhs.type.isFunction == rhs.type.isFunction &&
		lhs.type.lvlsOfIndirection == rhs.type.lvlsOfIndirection;
}

std::string intToStr(int i)
{
	char buf[256];
#ifdef _WIN32
	sprintf_s(buf, "%d", i);
#else
	snprintf(buf, 256, "%d", i);
#endif

	return string(buf);
}

bool GetSymbolType(const ASTNodeVal& node, Type* t, bool* isLvalue)
{
	if (node.type == ASTNodeValType::SYMBOL) {
		if (isLvalue) {
			*isLvalue = !node.symbol->second.type.isFunction && node.symbol->second.type.arraySize <= 0;
		}

		*t = node.symbol->second.type;
		return true;
	}

	return false;
}

bool GetVariantType(const ASTNodeVal& node, Type* t)
{
	if (node.type == ASTNodeValType::VARIANT) {
		Variant::VariantType type = node.variant.getType();

		if (type == Variant::STRING) {
			t->spec = Type::CHAR;
			t->lvlsOfIndirection = 1;
		} else if (type == Variant::CHAR) {
			t->spec = Type::INT;
		} else if (type == Variant::INT) {
			t->spec = Type::INT;
		} else if (type == Variant::LONGINT) {
			t->spec = Type::LONGINT;
		}

		return true;
	}

	return false;
}