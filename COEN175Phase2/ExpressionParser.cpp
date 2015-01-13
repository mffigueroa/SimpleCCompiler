#include <iostream>
#include <set>

using namespace std;

#include "Header.h"

class ParseLevelFunctor {
public:
	ParseLevelFunctor(const std::set<std::string>& operatorSet, ParseLevelFunctor* nextLevel)
		: m_operatorSet(operatorSet), m_nextLevel(nextLevel){}

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
	std::set<std::string>			m_operatorSet;
	ParseLevelFunctor*	m_nextLevel;
};

class PrefixParseLevelFunctor : public ParseLevelFunctor {
public:
	PrefixParseLevelFunctor(const std::set<std::string>& operatorSet, ParseLevelFunctor* nextLevel)
		: ParseLevelFunctor(operatorSet, nextLevel) {}
	virtual void operator()();
};

class ArrayRefParseLevelFunctor : public ParseLevelFunctor {
public:
	ArrayRefParseLevelFunctor(ParseLevelFunctor* nextLevel)
		: ParseLevelFunctor(std::set<string>(), nextLevel){}
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
			cout << *op << endl;
			(*m_nextLevel)();
	}
}

void PrefixParseLevelFunctor::operator()()
{
	for (set<string>::const_iterator op = m_operatorSet.find(lookahead());
		op != m_operatorSet.end(); op = m_operatorSet.find(lookahead())) {
			match(*op);
			cout << *op << endl;
	}

	(*m_nextLevel)();
}

void ArrayRefParseLevelFunctor::operator()()
{
	(*m_nextLevel)();

	while (lookahead() == "[") {
		match("[");
		(*m_nextLevel)();
		match("]");
		cout << "[]" << endl;
	}
}

void FinalParseLevelFunctor::operator()()
{
	if (lookahead() == "id") {
		match("id");
		cout << "id" << endl;
	} else {
		match("(");
		(*m_firstLevel)();
		match(")");
		cout << "()" << endl;
	}
}

ExpressionParser::ExpressionParser()
{
	FinalParseLevelFunctor		firstLevel(0);
	ArrayRefParseLevelFunctor*	secondLevel;
	secondLevel = new ArrayRefParseLevelFunctor(&firstLevel);

	set<string> operators;
	operators.insert("&");
	operators.insert("*");
	operators.insert("!");
	operators.insert("-");
	operators.insert("sizeof");
	PrefixParseLevelFunctor*	thirdLvl;
	thirdLvl = new PrefixParseLevelFunctor(operators, secondLevel);

	operators.clear();
	operators.insert("*");
	operators.insert("/");
	operators.insert("%");
	ParseLevelFunctor*			fourthLvl; 
	fourthLvl = new ParseLevelFunctor(operators, thirdLvl);

	operators.clear();
	operators.insert("+");
	operators.insert("-");
	ParseLevelFunctor*			fifthLvl;
	fifthLvl = new ParseLevelFunctor(operators, fourthLvl);

	operators.clear();
	operators.insert("<");
	operators.insert(">");
	operators.insert("<=");
	operators.insert(">=");
	ParseLevelFunctor*			sixthLvl;
	sixthLvl = new ParseLevelFunctor(operators, fifthLvl);

	operators.clear();
	operators.insert("==");
	operators.insert("!=");
	ParseLevelFunctor*			seventhLvl;
	seventhLvl = new ParseLevelFunctor(operators, sixthLvl);

	operators.clear();
	operators.insert("&&");
	ParseLevelFunctor*			eightLvl;
	eightLvl = new ParseLevelFunctor(operators, seventhLvl);

	operators.clear();
	operators.insert("||");
	m_ninthLvl = new ParseLevelFunctor(operators, eightLvl);

	firstLevel.SetFirstLevel(m_ninthLvl);
}

void ExpressionParser::operator()()
{
	(*m_ninthLvl)();
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
		if(lookahead == "id") {
			match("id");
		} else {
			match('(');
			A();
			match(')');
		}
	}

*/