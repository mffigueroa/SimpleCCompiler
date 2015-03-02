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
		: spec(INT), lvlsOfIndirection(0), arraySize(0), isFunction(false)
	{}

	bool operator==(const Type& rhs) const;
	bool operator!=(const Type& rhs) const;

	bool					isFunction;
	eSpecifier				spec;
	size_t					lvlsOfIndirection;
	size_t					arraySize;
	std::vector<Symbol*>	funcParams;
};

bool	isNumericType(const Type& t);
bool	isLogicalType(const Type& t);
bool	isPointerType(const Type& t);
bool	typesCompatible(const Type& t1, const Type& t2);
void	promoteType(Type& t);
size_t	GetTypeSize(const Type& t);

#endif