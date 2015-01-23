#ifndef __VARIANT_H__
#define __VARIANT_H__

#include <string>

#include "Type.h"

// supports:
//	string, longinteger, integer, character, identifier

class Variant
{
public:
	Variant();
	Variant(eSpecifier spec, const std::string& strVal);
	
	void operator=(const Variant& v);

	void setVal(eSpecifier spec, const std::string& strVal);
	eSpecifier getType() const;

	std::string	getStrVal() const;
	int			getIntVal() const;
	char		getCharVal() const;
private:
	std::string m_strVal;

	union
	{		
		int	 m_intVal;
		char m_charVal;
	};

	eSpecifier m_type;
};

#endif