#include <vector>

using namespace std;

#include "Type.h"
#include "Symbol.h"

bool Type::operator==(const Type& rhs)
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

bool Type::operator!=(const Type& rhs)
{
	return !(*this == rhs);
}