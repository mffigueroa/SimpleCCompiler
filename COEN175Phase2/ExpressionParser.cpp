#include <iostream>
#include <set>
#include <map>
#include <string>

using namespace std;

#include "Header.h"
#include "Tree.h"

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

	virtual TreeNode<Variant>* operator()(ScopeStack& variableStack, ScopeStack& functionStack);
protected:
	set<string>			m_operatorSet;
	map<string, string>	m_operatorNames;
	ParseLevelFunctor*	m_nextLevel;
};

class PrefixParseLevelFunctor : public ParseLevelFunctor {
public:
	PrefixParseLevelFunctor(const set<string>& operatorSet, const map<string, string>& operatorNames, ParseLevelFunctor* nextLevel)
		: ParseLevelFunctor(operatorSet, operatorNames, nextLevel) {}
	virtual TreeNode<Variant>* operator()(ScopeStack& variableStack, ScopeStack& functionStack);
};

class CastParseLevelFunctor : public ParseLevelFunctor {
public:
	CastParseLevelFunctor(ParseLevelFunctor* nextLevel)
		: ParseLevelFunctor(nextLevel) {}
	virtual TreeNode<Variant>* operator()(ScopeStack& variableStack, ScopeStack& functionStack);
};

class ArrayRefParseLevelFunctor : public ParseLevelFunctor {
public:
	ArrayRefParseLevelFunctor(ParseLevelFunctor* nextLevel)
		: ParseLevelFunctor(nextLevel){}
	virtual TreeNode<Variant>* operator()(ScopeStack& variableStack, ScopeStack& functionStack);
};

class FinalParseLevelFunctor : public ParseLevelFunctor {
public:
	FinalParseLevelFunctor(ParseLevelFunctor* firstLevel)
		: m_firstLevel(firstLevel) {}

	void SetFirstLevel(ParseLevelFunctor* firstLevel);

	virtual TreeNode<Variant>* operator()(ScopeStack& variableStack, ScopeStack& functionStack);
private:
	ParseLevelFunctor* m_firstLevel;
};

class ExpressionParser {
public:
	ExpressionParser();
	TreeNode<Variant>* operator()(ScopeStack& variableStack, ScopeStack& functionStack);
private:
	ParseLevelFunctor*		m_tenthLvl;
};

void FinalParseLevelFunctor::SetFirstLevel(ParseLevelFunctor* firstLevel)
{
	m_firstLevel = firstLevel;
}

TreeNode<Variant>* ParseLevelFunctor::operator()(ScopeStack& variableStack, ScopeStack& functionStack)
{
	TreeNode<Variant>* childNode = (*m_nextLevel)(variableStack, functionStack);
	TreeNode<Variant> *rootNode = 0, *parentNode = 0;

	for (set<string>::const_iterator op = m_operatorSet.find(lookahead());
		op != m_operatorSet.end(); op = m_operatorSet.find(lookahead())) {
			match(*op);

			if (!rootNode) {
				rootNode = new TreeNode<Variant>;
				rootNode->getVal().setVal(STRING, m_operatorNames[*op]);
				parentNode = rootNode;
				rootNode->addChild(childNode);
			} else {
				parentNode = new TreeNode<Variant>(parentNode);
				parentNode->getVal().setVal(STRING, m_operatorNames[*op]);
				parentNode->addChild(childNode);
			}

			childNode = (*m_nextLevel)(variableStack, functionStack);

			cout << m_operatorNames[*op] << endl;
	}

	if (parentNode) {
		parentNode->addChild(childNode);
	}

	if (rootNode) {
		return rootNode;
	} else {
		return childNode;
	}
}

TreeNode<Variant>* PrefixParseLevelFunctor::operator()(ScopeStack& variableStack, ScopeStack& functionStack)
{
	set<string>::const_iterator op = m_operatorSet.find(lookahead());

	if (op != m_operatorSet.end()) {
		match(*op);

		TreeNode<Variant>* rootNode = new TreeNode<Variant>;
		rootNode->getVal().setVal(STRING, m_operatorNames[*op]);

		if (*op == "sizeof" && lookahead() == "(") {
			match("(");
			eSpecifier spec = Specifier();
			Pointers();
			match(")");

			TreeNode<Variant>* node = new TreeNode<Variant>(rootNode);
			node->getVal().setVal(STRING, GetSpecifierName(spec));
		} else {
			rootNode->addChild((*this)(variableStack, functionStack));
		}

		cout << m_operatorNames[*op] << endl;

		return rootNode;
	} else {
		return (*m_nextLevel)(variableStack, functionStack);
	}
}

TreeNode<Variant>* CastParseLevelFunctor::operator()(ScopeStack& variableStack, ScopeStack& functionStack)
{
	if (lookahead() == "(" &&
		(lookahead(1) == "int" || lookahead(1) == "long" || lookahead(1) == "char")) {
			match("(");
			Specifier();
			Pointers();
			match(")");

			TreeNode<Variant>* rootNode = new TreeNode<Variant>;
			rootNode->getVal().setVal(STRING, "CAST");

			rootNode->addChild((*m_nextLevel)(variableStack, functionStack));
			cout << "cast" << endl;

			return rootNode;
	} else {
		return (*m_nextLevel)(variableStack, functionStack);
	}
}

TreeNode<Variant>* ArrayRefParseLevelFunctor::operator()(ScopeStack& variableStack, ScopeStack& functionStack)
{
	TreeNode<Variant>* rootNode = (*m_nextLevel)(variableStack, functionStack);
	TreeNode<Variant>* parentNode = rootNode;

	while (lookahead() == "[") {
		match("[");

		TreeNode<Variant>* childNode = new TreeNode<Variant>(parentNode);
		childNode->getVal().setVal(STRING, "index");
		childNode->addChild(Expression(variableStack, functionStack));
		cout << "index" << endl;
		match("]");
	}
	
	return rootNode;
}

TreeNode<Variant>* FinalParseLevelFunctor::operator()(ScopeStack& variableStack, ScopeStack& functionStack)
{
	string currToken = lookahead();

	if (currToken == "IDENTIFIER") {
		Variant v;
		unsigned int lineNumber;
		match("IDENTIFIER", &v, &lineNumber);

		

		TreeNode<Variant>* rootNode = new TreeNode<Variant>;
		rootNode->getVal() = v;

		if (lookahead() == "(") {
			match("(");

			TreeNode<Variant>* funcCallNode = new TreeNode<Variant>;
			funcCallNode->getVal().setVal(STRING, "function_call");
			funcCallNode->addChild(rootNode);

			// use of an undefined variable
			if (!LookupSymbol(functionStack, v.getStrVal())) {
				outputError(lineNumber, "'" + v.getStrVal() + "' undeclared");
			}

			if (lookahead() != ")") {
				list<TreeNode<Variant>*> nodes = ExpressionList(variableStack, functionStack);

				for (list<TreeNode<Variant>*>::const_iterator i = nodes.begin(), i_end = nodes.end();
					i != i_end; ++i) {
						funcCallNode->addChild(*i);
				}
			}

			match(")");
			return funcCallNode;
		} else {
			// use of an undefined variable
			if (!LookupSymbol(variableStack, v.getStrVal())) {
				outputError(lineNumber, "'" + v.getStrVal() + "' undeclared");
			}
		}

		return rootNode;
	} else if (currToken == "INTEGER" || currToken == "LONGINTEGER" || currToken == "STRING" || currToken == "CHARACTER") {
		Variant v;
		match(currToken, &v);

		TreeNode<Variant>* rootNode = new TreeNode<Variant>;
		rootNode->getVal() = v;
		return rootNode;
	} else {
		match("(");
		return (*m_firstLevel)(variableStack, functionStack);
		match(")");
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

TreeNode<Variant>* ExpressionParser::operator()(ScopeStack& variableStack, ScopeStack& functionStack)
{
	return (*m_tenthLvl)(variableStack, functionStack);
}

TreeNode<Variant>* Expression(ScopeStack& variableStack, ScopeStack& functionStack)
{
	static ExpressionParser expr;
	return expr(variableStack, functionStack);
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