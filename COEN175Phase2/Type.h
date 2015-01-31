#ifndef __TYPE_H__
#define __TYPE_H__

#include <vector>

class Symbol;

class Type {
public:
	typedef enum {
		INT = 1,
		LONGINT,
		CHAR,
		STRING,
		UNDEFINED
	} eSpecifier;

	Type()
		: spec(INT), lvlsOfIndirection(0)
	{}

	bool operator==(const Type& rhs);
	bool operator!=(const Type& rhs);

	bool					isFunction;
	eSpecifier				spec;
	size_t					lvlsOfIndirection;
	size_t					arraySize;
	std::vector<Symbol*>	funcParams;
};

#endif