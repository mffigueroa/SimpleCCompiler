#include "Header.h"

#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
using namespace std;

vector<pair<string, string>> tokens;

void error()
{
	cout << "Error" << endl;
	system("pause");
	exit(0);
}

void match(const string& tokenType)
{
	pair<string, string> currToken = *tokens.rbegin();
	tokens.pop_back();

	if (currToken.first != tokenType) {
		error();
	}
}

string lookahead()
{
	if (tokens.empty()) {
		return "";
	}

	return tokens.rbegin()->first;
}