#include <vector>

using namespace std;

#include "Type.h"
#include "Symbol.h"

bool Type::operator==(const Type& rhs) const
{
	if (isFunction == rhs.isFunction &&
		spec == rhs.spec &&
		lvlsOfIndirection == rhs.lvlsOfIndirection &&
		arraySize == rhs.arraySize &&
		funcParams.size() == rhs.funcParams.size()) {

		vector<Symbol*>::const_iterator i = funcParams.begin(), j = rhs.funcParams.begin();
		for (; i != funcParams.end() && j != rhs.funcParams.end(); ++i, ++j) {
			if (**i != **j) {
				return false;
			}
		}

		return true;
	}

	return false;
}

bool Type::operator!=(const Type& rhs) const
{
	return !(*this == rhs);
}

bool isNumericType(const Type& t)
{
	if (t.isFunction || t.arraySize > 0 || t.lvlsOfIndirection > 0) {
		return false;
	}

	if (t.spec == Type::INT || t.spec == Type::LONGINT) {
		return true;
	}

	// can be promoted to 'int'
	if (t.spec == Type::CHAR) {
		return true;
	}

	return false;
}

bool isLogicalType(const Type& t)
{
	if (t.isFunction) {
		return false;
	}

	if (isNumericType(t)) {
		return true;
	}

	// pointer to T
	if (isPointerType(t)) {
		return true;
	}

	return false;
}

bool isPointerType(const Type& t)
{
	if (t.isFunction) {
		return false;
	}

	if (t.lvlsOfIndirection > 0) {
		return true;
	}

	// can be promoted to 'pointer to T'
	if (t.arraySize > 0) {
		return true;
	}

	return false;
}

bool typesCompatible(const Type& t1, const Type& t2)
{
	if (isNumericType(t1) && isNumericType(t2)) {
		return true;
	}

	if (isLogicalType(t1) && isLogicalType(t2) && t1 == t2) {
		return true;
	}

	return false;
}

void promoteType(Type& t)
{
	if (t.isFunction) {
		return;
	}

	if (t.arraySize > 0) {
		t.arraySize = 0;
		++t.lvlsOfIndirection;
	}

	if (t.lvlsOfIndirection <= 0 && t.spec == Type::CHAR) {
		t.spec = Type::INT;
	}
}

size_t	GetTypeSize(const Type& t)
{
	size_t varSize;

	if (t.lvlsOfIndirection > 0) {
		// a pointer
		varSize = 8;
	}
	else if (t.spec == Type::LONGINT) {
		varSize = 8;
	}
	else if (t.spec == Type::INT) {
		varSize = 4;
	}
	else if (t.spec == Type::CHAR) {
		varSize = 1;
	}

	if (t.arraySize > 0) {
		varSize *= t.arraySize;
	}

	return varSize;
}