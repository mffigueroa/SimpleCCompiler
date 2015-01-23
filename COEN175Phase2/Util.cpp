#include "Header.h"
#include "Tokens.h"

#include <iostream>
#include <stdlib.h>
#include <deque>
#include <string>

extern int yylex (void);
extern Variant currVariant;

using namespace std;

deque<pair<int, Variant>>	tokenBuffer;
size_t		currTokenIndex;

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
	pair<int, Variant> currToken;

	if (tokenBuffer.size()) {
		currToken = tokenBuffer.front();
		tokenBuffer.pop_front();
	}
	else {
		currToken.first = yylex();
		currToken.second = currVariant;
	}

	if (currToken.first != tokenMap[tokenType]) {
		error();
	}

	v = currToken.second;
}

string lookahead(unsigned int ahead)
{
	if(ahead + 1 > tokenBuffer.size()) {
	  int tokensToRead = ahead - tokenBuffer.size() + 1;
	  
	  for(int i = 0; i < tokensToRead; ++i) {
		  int tok = yylex();
		  pair<int, Variant> currToken(tok, currVariant);
		  tokenBuffer.push_back(currToken);
	  }
	}

	deque<pair<int, Variant>>::const_iterator it = tokenBuffer.begin() + ahead;
	return reverseTokenMap[it->first];
}