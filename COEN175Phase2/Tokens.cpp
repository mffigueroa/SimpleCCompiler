#include <string>
#include <algorithm>
#include <map>

using namespace std;

#include "Tokens.h"

map<string, unsigned int> tokenMap;
map<unsigned int, string> reverseTokenMap;
 
void CreateTokenMap()
{
   string regexes[] = {
     "auto|break|case|char|const|continue|default|do|double|else|enum|extern|float|for|goto|if|int|long|register|return|short|signed|sizeof|static|struct|switch|typedef|union|unsigned|void|volatile|while",
     "=|\+\+|--|\|\||&&|==|!=|<|>|<=|>=|\+|-|\*|\/|%|&|!|->|\(|\)|\[|\]|\{|\}|;|:|\.|,"
   };
   
   unsigned int currTokenValue = 1;
   
   for(int i = 0; i < sizeof(regexes)/sizeof(string); ++i) {
     size_t startOfToken = 0;
     size_t endOfToken = regexes[i].find_first_of('|');
     
     for(; endOfToken != string::npos; endOfToken = regexes[i].find_first_of('|', startOfToken)) {
       if(regexes[i][startOfToken] != '|') {
	 tokenMap[regexes[i].substr(startOfToken, endOfToken - startOfToken)] = currTokenValue;
       } else {
	 endOfToken = regexes[i].find_first_not_of('|', startOfToken) - 1;
	 tokenMap[regexes[i].substr(startOfToken, endOfToken - startOfToken)] = currTokenValue;
       }
       ++currTokenValue;
       startOfToken = endOfToken + 1;
     }
     
     tokenMap[regexes[i].substr(startOfToken)] = currTokenValue;
     ++currTokenValue;
   }
   
   string remainingTokens[] = { "IDENTIFIER","INTEGER","LONGINTEGER","STRING","CHARACTER" };
   for(int i = 0; i < sizeof(remainingTokens)/sizeof(string); ++i) {
     tokenMap[remainingTokens[i]] = currTokenValue;
     ++currTokenValue;
   }
   
   for(map<string, unsigned int>::const_iterator it = tokenMap.begin(); it != tokenMap.end(); ++it) {
     reverseTokenMap[it->second] = it->first;
   }

   reverseTokenMap[0] = "EOF";
   tokenMap["EOF"] = 0;
}