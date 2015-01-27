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

void error()
{
	cout << "Error" << endl;
	exit(0);
}

void match(const string& tokenType)
{
	Variant throwAway;
	match(tokenType, throwAway);
}

void match(const string& tokenType, Variant& v)
{
	TokenTriple currToken;

	if (tokenBuffer.size()) {
		currToken = tokenBuffer.front();
		tokenBuffer.pop_front();
	} else {
		currToken.first = yylex();
		currToken.second.first = currVariant;
	}

	if (currToken.first != tokenMap[tokenType]) {
		error();
	}

	v = currToken.second.first;
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

void outputError(const std::string& err)
{
	cerr << err << endl;
}