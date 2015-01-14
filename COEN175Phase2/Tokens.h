#ifndef __TOKENS_H__
#define __TOKENS_H__

#include <map>
#include <string>

extern std::map<std::string, unsigned int> tokenMap;
extern std::map<unsigned int, std::string> reverseTokenMap;

void CreateTokenMap();

#endif