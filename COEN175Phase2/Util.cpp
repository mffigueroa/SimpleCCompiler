#include "Header.h"
#include "Tokens.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
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

string GetSpecifierName(Type::eSpecifier spec)
{	
	if(spec == Type::INT) {
		return "INT"; 
	} else if (spec == Type::LONGINT) {
		return "LONGINT";
	} else if (spec == Type::CHAR) {
		return "CHAR";
	} else if (spec == Type::STRING) {
		return "STRING";
	} else {
		return "UNDEFINED";
	}
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

void addChildrenToNode(TreeNode<ASTNodeVal>* v, const list<TreeNode<ASTNodeVal>*>& children)
{
	list<TreeNode<ASTNodeVal>*>::const_iterator it = children.begin(), it_end = children.end();

	for (; it != it_end; ++it) {
		v->addChild(*it);
	}
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