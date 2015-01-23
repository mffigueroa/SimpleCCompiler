#include <string>
using namespace std;

#include "Variant.h"

Variant::Variant()
	: m_type(UNDEFINED)
{}

Variant::Variant(eSpecifier spec, const string& strVal)
{
	setVal(spec, strVal);
}

void Variant::setVal(eSpecifier spec, const std::string& strVal)
{
	m_type = spec;

	if (spec == STRING || spec == IDENTIFIER) {
		m_strVal = strVal;
	} else if (spec == INT || spec == LONGINT) {
		m_intVal = 0;

		for (size_t i = 0; i < strVal.length() && strVal[i] != 'L'; ++i) {
			m_intVal = (int)(strVal[i] - '0') + m_intVal * 10;
		}
	} else if (spec == CHAR) {
		if (strVal[0] == '\\') {
			char escaped = strVal[1];
			if (escaped == 'a') {
				m_charVal = '\a';
			} else if (escaped == 'b') {
				m_charVal = '\b';
			} else if (escaped == 'f') {
				m_charVal = '\f';
			} else if (escaped == 'n') {
				m_charVal = '\n';
			} else if (escaped == 'r') {
				m_charVal = '\r';
			} else if (escaped == 't') {
				m_charVal = '\t';
			} else if (escaped == 'v') {
				m_charVal = '\v';
			} else if (escaped == '\\') {
				m_charVal = '\\';
			} else if (escaped == '\'') {
				m_charVal = '\'';
			} else if (escaped == '\"') {
				m_charVal = '\"';
			} else if (escaped == '?') {
				m_charVal = '\?';
			} else if (escaped == 'n') {
				m_charVal = 0;

				for (int i = 2; i < 4; ++i) {
					char c = strVal[i];
					if ('0' <= c && c <= '7') {
						m_charVal = (c - '0') + m_charVal * 8;
					}
				}
			} else if (escaped == 'x') {
				m_charVal = 0;

				for (int i = 2; i < 4; ++i) {
					char c = strVal[i];
					if ('0' <= c && c <= '9') {
						m_charVal = (c - '0') + m_charVal * 16;
					} else if ('a' <= c && c <= 'f') {
						m_charVal = (c - 'a' + 10) + m_charVal * 16;
					} else if ('A' <= c && c <= 'F') {
						m_charVal = (c - 'A' + 10) + m_charVal * 16;
					}
				}
			}
		}
	}
}

void Variant::operator=(const Variant& v)
{
	m_type = v.m_type;

	if (m_type == STRING || m_type == IDENTIFIER) {
		m_strVal = v.m_strVal;
	} else if (m_type == INT || m_type == LONGINT) {
		m_intVal = v.m_intVal;
	} else if (m_type == CHAR) {
		m_charVal = v.m_charVal;
	}
}

eSpecifier Variant::getType() const
{
	return m_type;
}

std::string	Variant::getStrVal() const
{
	return m_strVal;
}

int Variant::getIntVal() const
{
	return m_intVal;
}

char Variant::getCharVal() const
{
	return m_charVal;
}