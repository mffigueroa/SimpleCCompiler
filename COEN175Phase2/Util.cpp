#include "Header.h"

#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
using namespace std;

vector<pair<string, string>>	tokens;
size_t							currTokenIndex;

void error()
{
	cout << "Error" << endl;
	system("pause");
	exit(0);
}

void match(const string& tokenType)
{
	vector<pair<string, string>>::const_iterator currTokenIt = tokens.begin() + currTokenIndex;

	if (currTokenIt->first != tokenType) {
		error();
	}

	++currTokenIndex;
}

string lookahead(int ahead)
{
	if (tokens.empty()) {
		return "";
	}

	vector<pair<string, string>>::const_iterator it = tokens.begin() + currTokenIndex + ahead;
	return it->first;
}