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

string GetSpecifierName(eSpecifier spec)
{	
	if(spec == INT) {
		return "INT"; 
	} else if(spec == LONGINT) {
		return "LONGINT";
	} else if(spec == CHAR) {
		return "CHAR";
	} else if(spec == STRING) {
		return "STRING";
	} else if(spec == IDENTIFIER) {
		return "IDENTIFIER";
	} else {
		return "UNDEFINED";
	}
}

bool LookupSymbol(const ScopeStack& stack, const std::string& symbolName, Symbol* r_symbol)
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

void outputError(unsigned int lineNumber, const std::string& err)
{
	cerr << "line " << lineNumber << ": " << err << endl;
}