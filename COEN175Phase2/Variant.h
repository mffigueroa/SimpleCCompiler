#ifndef __VARIANT_H__
#define __VARIANT_H__

#include <string>

#include "Type.h"

// supports:
//	string, longinteger, integer, character, identifier

class Variant
{
public:
	typedef enum
	{
		INT = 1,
		LONGINT,
		CHAR,
		STRING,
		UNDEFINED,
		IDENTIFIER,
		SPECIFIER		
	} VariantType;

	Variant();
	Variant(VariantType type, const std::string& strVal);
	
	void operator=(const Variant& v);

	void setVal(VariantType spec, const std::string& strVal);
	void setVal(Type::eSpecifier spec);
	void setVal(int i);
	void setVal(char c);
	VariantType getType() const;

	Type::eSpecifier	getSpecifierVal() const;
	std::string	getSymbolVal() const;
	std::string	getStrVal() const;
	int			getIntVal() const;
	char		getCharVal() const;

private:
	std::string m_strVal;

	union
	{		
		int					m_intVal;
		char				m_charVal;
		Type::eSpecifier	m_specVal;
	};

	VariantType m_type;
};

#endif