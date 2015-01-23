#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include <string>

#include "Type.h"

class Symbol
{
public:
	Symbol(){}
	std::string identifier;
	Type		type;
};

#endif