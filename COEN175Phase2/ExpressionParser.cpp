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

	virtual TreeNode<ASTNodeVal>* operator()(ParserState& parserState);
protected:
	set<string>			m_operatorSet;
	map<string, string>	m_operatorNames;
	ParseLevelFunctor*	m_nextLevel;
};

class PrefixParseLevelFunctor : public ParseLevelFunctor {
public:
	PrefixParseLevelFunctor(const set<string>& operatorSet, const map<string, string>& operatorNames, ParseLevelFunctor* nextLevel)
		: ParseLevelFunctor(operatorSet, operatorNames, nextLevel) {}
	virtual TreeNode<ASTNodeVal>* operator()(ParserState& parserState);
};

class CastParseLevelFunctor : public ParseLevelFunctor {
public:
	CastParseLevelFunctor(ParseLevelFunctor* nextLevel)
		: ParseLevelFunctor(nextLevel) {}
	virtual TreeNode<ASTNodeVal>* operator()(ParserState& parserState);
};

class ArrayRefParseLevelFunctor : public ParseLevelFunctor {
public:
	ArrayRefParseLevelFunctor(ParseLevelFunctor* nextLevel)
		: ParseLevelFunctor(nextLevel){}
	virtual TreeNode<ASTNodeVal>* operator()(ParserState& parserState);
};

class FinalParseLevelFunctor : public ParseLevelFunctor {
public:
	FinalParseLevelFunctor(ParseLevelFunctor* firstLevel)
		: m_firstLevel(firstLevel) {}

	void SetFirstLevel(ParseLevelFunctor* firstLevel);

	virtual TreeNode<ASTNodeVal>* operator()(ParserState& parserState);
private:
	ParseLevelFunctor* m_firstLevel;
};

class ExpressionParser {
public:
	ExpressionParser();
	TreeNode<ASTNodeVal>* operator()(ParserState& parserState);
private:
	ParseLevelFunctor*		m_tenthLvl;
};

void FinalParseLevelFunctor::SetFirstLevel(ParseLevelFunctor* firstLevel)
{
	m_firstLevel = firstLevel;
}

TreeNode<ASTNodeVal>* ParseLevelFunctor::operator()(ParserState& parserState)
{
	TreeNode<ASTNodeVal>* childNode = (*m_nextLevel)(parserState);
	TreeNode<ASTNodeVal> *rootNode = 0, *parentNode = 0;

	for (set<string>::const_iterator op = m_operatorSet.find(lookahead());
		op != m_operatorSet.end(); op = m_operatorSet.find(lookahead())) {
			match(*op);

			if (!rootNode) {
				rootNode = new TreeNode<ASTNodeVal>;
				rootNode->val.variant.setVal(Variant::STRING, m_operatorNames[*op]);
				parentNode = rootNode;
				rootNode->addChild(childNode);
			} else {
				parentNode = new TreeNode<ASTNodeVal>(parentNode);
				parentNode->val.variant.setVal(Variant::STRING, m_operatorNames[*op]);
				parentNode->addChild(childNode);
			}

			childNode = (*m_nextLevel)(parserState);

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

TreeNode<ASTNodeVal>* PrefixParseLevelFunctor::operator()(ParserState& parserState)
{
	set<string>::const_iterator op = m_operatorSet.find(lookahead());

	if (op != m_operatorSet.end()) {
		match(*op);

		TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;
		rootNode->val.variant.setVal(Variant::STRING, m_operatorNames[*op]);

		if (*op == "sizeof" && lookahead() == "(") {
			match("(");
			Type::eSpecifier spec = Specifier();
			int indirectionLvl = Pointers();
			match(")");

			TreeNode<ASTNodeVal>* specNode = new TreeNode<ASTNodeVal>(rootNode);
			specNode->val.variant.setVal(spec);

			TreeNode<ASTNodeVal>* ptrNode = new TreeNode<ASTNodeVal>(rootNode);
			ptrNode->val.variant.setVal(indirectionLvl);
		} else {
			rootNode->addChild((*this)(parserState));
		}

		cout << m_operatorNames[*op] << endl;

		return rootNode;
	} else {
		return (*m_nextLevel)(parserState);
	}
}

TreeNode<ASTNodeVal>* CastParseLevelFunctor::operator()(ParserState& parserState)
{
	if (lookahead() == "(" && isSpecifier(lookahead(1))) {
			match("(");
			Type::eSpecifier spec = Specifier();
			int indirectionLvl = Pointers();
			match(")");

			TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;
			rootNode->val.variant.setVal(Variant::STRING, "CAST");

			TreeNode<ASTNodeVal>* specNode = new TreeNode<ASTNodeVal>(rootNode);
			specNode->val.variant.setVal(spec);

			TreeNode<ASTNodeVal>* ptrNode = new TreeNode<ASTNodeVal>(rootNode);
			specNode->val.variant.setVal(indirectionLvl);

			rootNode->addChild((*m_nextLevel)(parserState));
			cout << "cast" << endl;

			return rootNode;
	} else {
		return (*m_nextLevel)(parserState);
	}
}

TreeNode<ASTNodeVal>* ArrayRefParseLevelFunctor::operator()(ParserState& parserState)
{
	TreeNode<ASTNodeVal>* rootNode = (*m_nextLevel)(parserState);
	TreeNode<ASTNodeVal>* parentNode = rootNode;

	while (lookahead() == "[") {
		match("[");

		TreeNode<ASTNodeVal>* childNode = new TreeNode<ASTNodeVal>(parentNode);
		childNode->val.variant.setVal(Variant::STRING, "INDEX");
		childNode->addChild(Expression(parserState));
		cout << "index" << endl;
		match("]");
	}
	
	return rootNode;
}

TreeNode<ASTNodeVal>* FinalParseLevelFunctor::operator()(ParserState& parserState)
{
	string currToken = lookahead();

	if (currToken == "IDENTIFIER") {
		Variant v;
		unsigned int lineNumber;
		match("IDENTIFIER", &v, &lineNumber);		

		TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;

		if (lookahead() == "(") {
			match("(");

			SymbolTableRef sym;

			// calling an undefined function
			if (!LookupSymbol(parserState.functionStack, v.getStrVal(), &sym)) {
				outputError(lineNumber, "'" + v.getStrVal() + "' undeclared");
			}

			rootNode->val.symbol = sym;
			rootNode->val.isSymbol = true;

			TreeNode<ASTNodeVal>* funcCallNode = new TreeNode<ASTNodeVal>;
			funcCallNode->val.variant.setVal(Variant::STRING, "FUNC_CALL");
			funcCallNode->addChild(rootNode);

			if (lookahead() != ")") {
				list<TreeNode<ASTNodeVal>*> nodes = ExpressionList(parserState);

				for (list<TreeNode<ASTNodeVal>*>::const_iterator i = nodes.begin(), i_end = nodes.end();
					i != i_end; ++i) {
						funcCallNode->addChild(*i);
				}
			}

			match(")");
			return funcCallNode;
		} else {
			SymbolTableRef sym;

			// use of an undefined variable
			if (!LookupSymbol(parserState.variableStack, v.getStrVal(), &sym)) {
				outputError(lineNumber, "'" + v.getStrVal() + "' undeclared");
			}

			rootNode->val.symbol = sym;
			rootNode->val.isSymbol = true;

			return rootNode;
		}
	} else if (currToken == "INTEGER" || currToken == "LONGINTEGER" || currToken == "STRING" || currToken == "CHARACTER") {
		Variant v;
		match(currToken, &v);

		TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;
		rootNode->val.variant = v;
		return rootNode;
	} else {
		match("(");
		return (*m_firstLevel)(parserState);
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

TreeNode<ASTNodeVal>* ExpressionParser::operator()(ParserState& parserState)
{
	return (*m_tenthLvl)(parserState);
}

TreeNode<ASTNodeVal>* Expression(ParserState& parserState)
{
	static ExpressionParser expr;
	return expr(parserState);
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