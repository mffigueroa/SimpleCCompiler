#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include <string>

#include "Type.h"

class Symbol
{
public:
	Symbol() : defined(false) {}
	std::string identifier;
	Type		type;
	bool		defined;

	bool operator==(const Symbol& rhs);
	bool operator!=(const Symbol& rhs);
};

#endif