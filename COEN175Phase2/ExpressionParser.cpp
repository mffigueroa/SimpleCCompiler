#include <iostream>
#include <set>
#include <map>

using namespace std;

#include "Header.h"

class ParseLevelFunctor {
public:
	ParseLevelFunctor(const set<string>& operatorSet, const map<string, string>& operatorNames, ParseLevelFunctor* nextLevel)
		: m_operatorSet(operatorSet), m_operatorNames(operatorNames), m_nextLevel(nextLevel){}

	ParseLevelFunctor(ParseLevelFunctor* nextLevel)
		: m_nextLevel(nextLevel){}

	ParseLevelFunctor()
		: m_nextLevel(0) {}

	virtual ~ParseLevelFunctor()
	{
		if (m_nextLevel) {
			delete m_nextLevel;
		}
	}

	virtual void operator()();
protected:
	set<string>			m_operatorSet;
	map<string, string>	m_operatorNames;
	ParseLevelFunctor*	m_nextLevel;
};

class PrefixParseLevelFunctor : public ParseLevelFunctor {
public:
	PrefixParseLevelFunctor(const set<string>& operatorSet, const map<string, string>& operatorNames, ParseLevelFunctor* nextLevel)
		: ParseLevelFunctor(operatorSet, operatorNames, nextLevel) {}
	virtual void operator()();
};

class CastParseLevelFunctor : public ParseLevelFunctor {
public:
	CastParseLevelFunctor(ParseLevelFunctor* nextLevel)
		: ParseLevelFunctor(nextLevel) {}
	virtual void operator()();
};

class ArrayRefParseLevelFunctor : public ParseLevelFunctor {
public:
	ArrayRefParseLevelFunctor(ParseLevelFunctor* nextLevel)
		: ParseLevelFunctor(nextLevel){}
	virtual void operator()();
};

class FinalParseLevelFunctor : public ParseLevelFunctor {
public:
	FinalParseLevelFunctor(ParseLevelFunctor* firstLevel)
		: m_firstLevel(firstLevel) {}

	void SetFirstLevel(ParseLevelFunctor* firstLevel);

	virtual void operator()();
private:
	ParseLevelFunctor* m_firstLevel;
};

class ExpressionParser {
public:
	ExpressionParser();
	void operator()();
private:
	ParseLevelFunctor*		m_tenthLvl;
};

void FinalParseLevelFunctor::SetFirstLevel(ParseLevelFunctor* firstLevel)
{
	m_firstLevel = firstLevel;
}

void ParseLevelFunctor::operator()()
{
	(*m_nextLevel)();

	for (set<string>::const_iterator op = m_operatorSet.find(lookahead());
		op != m_operatorSet.end(); op = m_operatorSet.find(lookahead())) {
			match(*op);
			(*m_nextLevel)();
			cout << m_operatorNames[*op] << endl;
	}
}

void PrefixParseLevelFunctor::operator()()
{
	for (set<string>::const_iterator op = m_operatorSet.find(lookahead());
		op != m_operatorSet.end(); op = m_operatorSet.find(lookahead())) {
			match(*op);

			if (*op == "sizeof" && lookahead() == "(") {
				match("(");
				Specifier();
				Pointers();
				match(")");
			}

			cout << m_operatorNames[*op] << endl;
	}

	(*m_nextLevel)();
}

void CastParseLevelFunctor::operator()()
{
	if (lookahead() == "(" &&
		(lookahead(1) == "int" || lookahead(1) == "long" || lookahead(1) == "char")) {
			match("(");
			Specifier();
			Pointers();
			match(")");

			(*m_nextLevel)();
			cout << "cast" << endl;
	} else {
		(*m_nextLevel)();
	}
}

void ArrayRefParseLevelFunctor::operator()()
{
	(*m_nextLevel)();

	while (lookahead() == "[") {
		match("[");
		Expression();
		cout << "index" << endl;
		match("]");
	}
}

void FinalParseLevelFunctor::operator()()
{
	string currToken = lookahead();

	if (currToken == "IDENTIFIER") {
		match("IDENTIFIER");

		if (lookahead() == "(") {
			match("(");

			if (lookahead() != ")") {
				ExpressionList();
			}

			match(")");
		}
	} else if (currToken == "INTEGER" || currToken == "LONGINTEGER" || currToken == "STRING" || currToken == "CHARACTER") {
		match(currToken);
	} else {
		match("(");
		(*m_firstLevel)();
		match(")");
		cout << "()" << endl;
	}
}

ExpressionParser::ExpressionParser()
{
	FinalParseLevelFunctor*		firstLevel(0);
	firstLevel = new FinalParseLevelFunctor(0);
	ArrayRefParseLevelFunctor*	secondLevel;
	secondLevel = new ArrayRefParseLevelFunctor(firstLevel);

	set<string> operators;
	map<string, string> operatorOutputMap;
	operators.insert("&");
	operatorOutputMap["&"] = "addr";
	operators.insert("*");
	operatorOutputMap["*"] = "deref";
	operators.insert("!");
	operatorOutputMap["!"] = "not";
	operators.insert("-");
	operatorOutputMap["-"] = "neg";
	operators.insert("sizeof");
	operatorOutputMap["sizeof"] = "sizeof";
	PrefixParseLevelFunctor*	thirdLvl;
	thirdLvl = new PrefixParseLevelFunctor(operators, operatorOutputMap, secondLevel);

	CastParseLevelFunctor* fourthLvl;
	fourthLvl = new CastParseLevelFunctor(thirdLvl);

	operators.clear();
	operatorOutputMap.clear();
	operators.insert("*");
	operatorOutputMap["*"] = "mul";
	operators.insert("/");
	operatorOutputMap["/"] = "div";
	operators.insert("%");
	operatorOutputMap["%"] = "rem";
	ParseLevelFunctor*			fifthLvl; 
	fifthLvl = new ParseLevelFunctor(operators, operatorOutputMap, fourthLvl);

	operators.clear();
	operatorOutputMap.clear();
	operators.insert("+");
	operatorOutputMap["+"] = "add";
	operators.insert("-");
	operatorOutputMap["-"] = "sub";
	ParseLevelFunctor*			sixthLvl;
	sixthLvl = new ParseLevelFunctor(operators, operatorOutputMap, fifthLvl);

	operators.clear();
	operatorOutputMap.clear();
	operators.insert("<");
	operatorOutputMap["<"] = "ltn";
	operators.insert(">");
	operatorOutputMap[">"] = "gtn";
	operators.insert("<=");
	operatorOutputMap["<="] = "leq";
	operators.insert(">=");
	operatorOutputMap[">="] = "geq";
	ParseLevelFunctor*			seventhLvl;
	seventhLvl = new ParseLevelFunctor(operators, operatorOutputMap, sixthLvl);

	operators.clear();
	operatorOutputMap.clear();
	operators.insert("==");
	operatorOutputMap["=="] = "eql";
	operators.insert("!=");
	operatorOutputMap["!="] = "neq";
	ParseLevelFunctor*			eighthLvl;
	eighthLvl = new ParseLevelFunctor(operators, operatorOutputMap, seventhLvl);

	operators.clear();
	operatorOutputMap.clear();
	operators.insert("&&");
	operatorOutputMap["&&"] = "and";
	ParseLevelFunctor*			ninthLvl;
	ninthLvl = new ParseLevelFunctor(operators, operatorOutputMap, eighthLvl);

	operators.clear();
	operatorOutputMap.clear();
	operators.insert("||");
	operatorOutputMap["||"] = "or";
	m_tenthLvl = new ParseLevelFunctor(operators, operatorOutputMap, ninthLvl);

	firstLevel->SetFirstLevel(m_tenthLvl);
}

void ExpressionParser::operator()()
{
	(*m_tenthLvl)();
}

void Expression()
{
	static ExpressionParser expr;
	expr();
}

/*

	B();

	while (lookahead() == "||") {
		match("||");
		B();
	}

	void H() {
		while(lookahead in &, ..., sizeof) {
			match(lookahead);
		}
		I();
	}

	void I() {
		J();

		while(lookahead == '[') {
			match('[');
			J();
			match(']');
		}
	}

	void J() {
		if(lookahead == "IDENTIFIER") {
			match("IDENTIFIER");
		} else {
			match('(');
			A();
			match(')');
		}
	}

*/