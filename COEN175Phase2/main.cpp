#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
using namespace std;

#include "Header.h"

int main()
{
	typedef pair<string, string> token;
	tokens.push_back(token("id", "z"));
	tokens.push_back(token("*", " "));
	tokens.push_back(token("id", "y"));
	tokens.push_back(token("+", " "));
	tokens.push_back(token("id", "x"));

	ExpressionParser expr;
	expr();
	system("pause");
}

