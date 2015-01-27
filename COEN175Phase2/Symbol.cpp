#include "Symbol.h"

bool Symbol::operator==(const Symbol& rhs)
{
	return identifier == rhs.identifier && type == rhs.type;
}

bool Symbol::operator!=(const Symbol& rhs)
{
	return !(*this == rhs);
}