#include "Header.h"
#include "Tokens.h"

#include <iostream>
#include <stdlib.h>
#include <deque>
#include <string>

extern int yylex (void);

using namespace std;

deque<int>	tokenBuffer;
size_t		currTokenIndex;

void error()
{
	cout << "Error" << endl;
	exit(0);
}

void match(const string& tokenType)
{
	int currToken;
	
	if(tokenBuffer.size()) {
	  currToken = tokenBuffer.front();
	  tokenBuffer.pop_front();
	} else {
	  currToken = yylex();
	}

	if (currToken != tokenMap[tokenType]) {
		error();
	}
}

string lookahead(unsigned int ahead)
{
	if(ahead + 1 > tokenBuffer.size()) {
	  int tokensToRead = ahead - tokenBuffer.size() + 1;
	  
	  for(int i = 0; i < tokensToRead; ++i) {
	    tokenBuffer.push_back(yylex());
	  }
	}

	deque<int>::const_iterator it = tokenBuffer.begin() + ahead;
	return reverseTokenMap[*it];
}