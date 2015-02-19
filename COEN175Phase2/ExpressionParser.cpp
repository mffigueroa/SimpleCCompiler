#include <assert.h>
#include <iostream>
#include <set>
#include <map>
#include <string>

using namespace std;

#include "Header.h"
#include "Tree.h"
#include "Type.h"

typedef void(*TypeResolver)(TreeNode<ASTNodeVal>*, const string&);

void AdditiveExpTypeResolver(TreeNode<ASTNodeVal>* node, const string& op);
void MultiplicativeExpTypeResolver(TreeNode<ASTNodeVal>* node, const string& op);
void LogicalExpTypeResolver(TreeNode<ASTNodeVal>* node, const string& op);
void EqualityAndRelationalExpTypeResolver(TreeNode<ASTNodeVal>* node, const string& op);
void DerefTypeResolver(TreeNode<ASTNodeVal>* node, const string& op);
void AddrOfTypeResolver(TreeNode<ASTNodeVal>* node, const string& op);
void NotTypeResolver(TreeNode<ASTNodeVal>* node, const string& op);
void NegTypeResolver(TreeNode<ASTNodeVal>* node, const string& op);
void SizeOfTypeResolver(TreeNode<ASTNodeVal>* node, const string& op);
void IndexTypeResolver(TreeNode<ASTNodeVal>* node, const string& op);

void	GetBinaryChildrenTypes(TreeNode<ASTNodeVal>* node, Type* childArr);
void	GetUnaryChildrenType(TreeNode<ASTNodeVal>* node, Type* childType);

vector<string>	operatorNames;

class ParseLevelFunctor {
public:
	ParseLevelFunctor(const set<string>& operatorSet, const map<string, string>& operatorNames, const map<string, TypeResolver>& operatorTypeResolvers, ParseLevelFunctor* nextLevel)
		: m_operatorSet(operatorSet), m_operatorNames(operatorNames), m_operatorTypeResolvers(operatorTypeResolvers), m_nextLevel(nextLevel){}

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
	set<string>					m_operatorSet;
	map<string, string>			m_operatorNames;
	map<string, TypeResolver>	m_operatorTypeResolvers;
	ParseLevelFunctor*			m_nextLevel;
};

class PrefixParseLevelFunctor : public ParseLevelFunctor {
public:
	PrefixParseLevelFunctor(const set<string>& operatorSet, const map<string, string>& operatorNames, const map<string, TypeResolver>& operatorTypeResolvers, ParseLevelFunctor* nextLevel)
		: ParseLevelFunctor(operatorSet, operatorNames, operatorTypeResolvers, nextLevel) {}
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

	if (!childNode) {
		return NULL;
	}

	TreeNode<ASTNodeVal> *rootNode = 0;
	set<string>::const_iterator op;
	string opStr;

	for (op = m_operatorSet.find(lookahead()); op != m_operatorSet.end(); op = m_operatorSet.find(lookahead())) {
		unsigned int lineNumber = 0;
		opStr = *op;
		match(opStr, NULL, &lineNumber);

		if (rootNode) {
			rootNode->addChild(childNode);
			m_operatorTypeResolvers[opStr](rootNode, opStr);
			childNode = rootNode;
		}

		rootNode = new TreeNode<ASTNodeVal>;
		rootNode->val.type = ASTNodeValType::VARIANTTYPENODE;
		rootNode->val.variantTypeNode.variant.setVal(Variant::STRING, m_operatorNames[opStr]);
		rootNode->addChild(childNode);

		rootNode->val.lineNumber = lineNumber;

		childNode = (*m_nextLevel)(parserState);

		if (!childNode) {
			if (rootNode) {
				delete rootNode;
			}

			return NULL;
		}

		cout << m_operatorNames[opStr] << endl;
	}

	if (rootNode) {
		rootNode->addChild(childNode);
		m_operatorTypeResolvers[opStr](rootNode, opStr);
		return rootNode;
	} else {
		return childNode;
	}
}

TreeNode<ASTNodeVal>* PrefixParseLevelFunctor::operator()(ParserState& parserState)
{
	set<string>::const_iterator op = m_operatorSet.find(lookahead());

	if (op != m_operatorSet.end()) {
		unsigned int lineNumber = 0;
		match(*op, NULL, &lineNumber);

		TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;
		rootNode->val.type = ASTNodeValType::VARIANTTYPENODE;
		rootNode->val.variantTypeNode.variant.setVal(Variant::STRING, m_operatorNames[*op]);
		rootNode->val.lineNumber = lineNumber;

		if (*op == "sizeof" && lookahead() == "(" && isSpecifier(lookahead(1))) {
			match("(");
			Type::eSpecifier spec = Specifier();
			int indirectionLvl = Pointers();
			match(")");

			TreeNode<ASTNodeVal>* specNode = new TreeNode<ASTNodeVal>(rootNode);
			specNode->val.type = ASTNodeValType::VARIANT;
			specNode->val.variant.setVal(spec);

			TreeNode<ASTNodeVal>* ptrNode = new TreeNode<ASTNodeVal>(rootNode);
			ptrNode->val.type = ASTNodeValType::VARIANT;
			ptrNode->val.variant.setVal(indirectionLvl);

			rootNode->val.variantTypeNode.isLvalue = false;
			rootNode->val.variantTypeNode.type.spec = Type::LONGINT;
		} else {
			TreeNode<ASTNodeVal>* child = (*this)(parserState);

			if (!child) {
				return NULL;
			}

			rootNode->addChild(child);
			m_operatorTypeResolvers[*op](rootNode, *op);
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
			unsigned int lineNumber = 0;
			match("(", NULL, &lineNumber);
			
			Type::eSpecifier spec = Specifier();
			int indirectionLvl = Pointers();
			match(")");

			TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;
			rootNode->val.type = ASTNodeValType::VARIANTTYPENODE;
			rootNode->val.variantTypeNode.variant.setVal(Variant::STRING, "CAST");
			rootNode->val.lineNumber = lineNumber;

			TreeNode<ASTNodeVal>* specNode = new TreeNode<ASTNodeVal>(rootNode);
			specNode->val.type = ASTNodeValType::VARIANT;
			specNode->val.variant.setVal(spec);

			TreeNode<ASTNodeVal>* ptrNode = new TreeNode<ASTNodeVal>(rootNode);
			ptrNode->val.type = ASTNodeValType::VARIANT;
			ptrNode->val.variant.setVal(indirectionLvl);

			TreeNode<ASTNodeVal>* child = (*this)(parserState);

			if (!child) {
				delete rootNode;
				return NULL;
			}

			rootNode->addChild(child);
			cout << "cast" << endl;

			Type castType;
			castType.spec = spec;
			castType.lvlsOfIndirection = indirectionLvl;

			const Type* childType;

			if (child->val.type == ASTNodeValType::VARIANTTYPENODE) {
				childType = &child->val.variantTypeNode.type;
			} else {
				childType = &child->val.symbol->second.type;
			}

			if (childType->isFunction) {
				outputError(lineNumber, "invalid operand in cast expression");
				return NULL;
			}

			bool numToNum = isNumericType(castType) && isNumericType(*childType);
			bool ptrToPtr = isPointerType(castType) && isPointerType(*childType);

			if (!numToNum && !ptrToPtr) {
				bool castToLong = castType.lvlsOfIndirection <= 0 &&
									castType.arraySize <= 0 &&
									castType.spec == Type::LONGINT;
				bool castFromLong = childType->lvlsOfIndirection <= 0 &&
									childType->arraySize <= 0 &&
									childType->spec == Type::LONGINT;
				bool castToPtr = isPointerType(castType);
				bool castFromPtr = isPointerType(*childType);

				if (!(castFromPtr && castToLong) && !(castFromLong && castToPtr)) {
					outputError(lineNumber, "invalid operand in cast expression");
					return NULL;
				}
			}

			rootNode->val.variantTypeNode.type = castType;

			return rootNode;
	} else {
		return (*m_nextLevel)(parserState);
	}
}

TreeNode<ASTNodeVal>* ArrayRefParseLevelFunctor::operator()(ParserState& parserState)
{
	TreeNode<ASTNodeVal>* rootNode = (*m_nextLevel)(parserState);

	if (!rootNode) {
		return NULL;
	}

	while (lookahead() == "[") {
		unsigned int lineNumber;
		match("[", NULL, &lineNumber);

		TreeNode<ASTNodeVal>* node = new TreeNode<ASTNodeVal>;
		node->val.type = ASTNodeValType::VARIANTTYPENODE;
		node->val.variantTypeNode.variant.setVal(Variant::STRING, "INDEX");
		node->val.lineNumber = lineNumber;

		node->addChild(rootNode);
		rootNode = node;

		TreeNode<ASTNodeVal>* indexNode = Expression(parserState);

		if (!indexNode) {
			return NULL;
		}

		rootNode->addChild(indexNode);

		IndexTypeResolver(rootNode, "[]");

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
			if (!LookupSymbol(parserState.stack, v.getStrVal(), &sym)) {
				outputError(lineNumber, "'" + v.getStrVal() + "' undeclared");
				delete rootNode;
				return NULL;
			}

			rootNode->val.symbol = sym;
			rootNode->val.type = ASTNodeValType::SYMBOL;

			if (!sym->second.type.isFunction) {
				outputError(lineNumber, "called object is not a function");
				return NULL;
			}

			TreeNode<ASTNodeVal>* funcCallNode = new TreeNode<ASTNodeVal>;
			funcCallNode->val.type = ASTNodeValType::VARIANTTYPENODE;
			funcCallNode->val.variantTypeNode.variant.setVal(Variant::STRING, "FUNC_CALL");
			funcCallNode->val.lineNumber = lineNumber;
			funcCallNode->addChild(rootNode);

			if (lookahead() != ")") {
				if (!addChildrenToNode(funcCallNode, ExpressionList(parserState))) {
					return NULL;
				}
			}

			match(")");

			// we don't have function parameters if
			// its not defined.
			if (sym->second.defined) {
				const list<TreeNode<ASTNodeVal>*>& children = funcCallNode->getChildList();

				// incorrect number of parameters in call
				if (children.size() - 1 != sym->second.type.funcParams.size()) {
					outputError(lineNumber, "invalid arguments to called function");
					return NULL;
				}

				vector<Symbol*>::const_iterator param = sym->second.type.funcParams.begin();
				for (list<TreeNode<ASTNodeVal>*>::const_iterator i = ++children.begin(), i_end = children.end();
					i != i_end; ++i, ++param) {
						Type t;
						Type* tPtr = &t;
						if (!GetSymbolType((*i)->val, &t)) {
							if (!GetVariantType((*i)->val, &t)) {
								tPtr = &(*i)->val.variantTypeNode.type;
							}
						}

						if (!typesCompatible(*tPtr, (*param)->type)) {
							outputError(lineNumber, "invalid arguments to called function");
							return NULL;
						}
				}
			}

			funcCallNode->val.variantTypeNode.type.arraySize = sym->second.type.arraySize;
			funcCallNode->val.variantTypeNode.type.lvlsOfIndirection = sym->second.type.lvlsOfIndirection;
			funcCallNode->val.variantTypeNode.type.spec = sym->second.type.spec;
			funcCallNode->val.variantTypeNode.type.isFunction = false;
			funcCallNode->val.variantTypeNode.isLvalue = false;

			return funcCallNode;
		} else {
			SymbolTableRef sym;

			// use of an undefined variable
			if (!LookupSymbol(parserState.stack, v.getStrVal(), &sym)) {
				outputError(lineNumber, "'" + v.getStrVal() + "' undeclared");
				delete rootNode;
				return NULL;
			}

			rootNode->val.symbol = sym;
			rootNode->val.type = ASTNodeValType::SYMBOL;
			rootNode->val.lineNumber = lineNumber;

			return rootNode;
		}
	} else if (currToken == "INTEGER" || currToken == "LONGINTEGER" || currToken == "STRING" || currToken == "CHARACTER") {
		Variant v;
		unsigned int lineNumber;
		match(currToken, &v, &lineNumber);

		TreeNode<ASTNodeVal>* rootNode = new TreeNode<ASTNodeVal>;
		rootNode->val.variant = v;
		rootNode->val.type = ASTNodeValType::VARIANT;
		rootNode->val.lineNumber = lineNumber;
		return rootNode;
	} else {
		match("(");
		TreeNode<ASTNodeVal>* node = (*m_firstLevel)(parserState);
		match(")");
		return node;
	}
}

ExpressionParser::ExpressionParser()
{
	FinalParseLevelFunctor*		firstLevel(0);
	firstLevel = new FinalParseLevelFunctor(0);
	ArrayRefParseLevelFunctor*	secondLevel;
	secondLevel = new ArrayRefParseLevelFunctor(firstLevel);

	operatorNames.clear();
	operatorNames.push_back("addr");
	operatorNames.push_back("deref");
	operatorNames.push_back("not");
	operatorNames.push_back("neg");
	operatorNames.push_back("sizeof");
	operatorNames.push_back("mul");
	operatorNames.push_back("div");
	operatorNames.push_back("rem");
	operatorNames.push_back("add");
	operatorNames.push_back("sub");
	operatorNames.push_back("ltn");
	operatorNames.push_back("gtn");
	operatorNames.push_back("leq");
	operatorNames.push_back("geq");
	operatorNames.push_back("eql");
	operatorNames.push_back("neq");
	operatorNames.push_back("and");
	operatorNames.push_back("or");

	set<string> operators;
	map<string, string> operatorOutputMap;
	map<string, TypeResolver> operatorTypeResolverMap;
	operators.insert("&");
	operatorOutputMap["&"] = "addr";
	operatorTypeResolverMap["&"] = AddrOfTypeResolver;
	operators.insert("*");
	operatorOutputMap["*"] = "deref";
	operatorTypeResolverMap["*"] = DerefTypeResolver;
	operators.insert("!");
	operatorOutputMap["!"] = "not";
	operatorTypeResolverMap["!"] = NotTypeResolver;
	operators.insert("-");
	operatorOutputMap["-"] = "neg";
	operatorTypeResolverMap["-"] = NegTypeResolver;
	operators.insert("sizeof");
	operatorOutputMap["sizeof"] = "sizeof";
	operatorTypeResolverMap["sizeof"] = SizeOfTypeResolver;

	PrefixParseLevelFunctor*	thirdLvl;
	thirdLvl = new PrefixParseLevelFunctor(operators, operatorOutputMap, operatorTypeResolverMap, secondLevel);

	CastParseLevelFunctor* fourthLvl;
	fourthLvl = new CastParseLevelFunctor(thirdLvl);

	operators.clear();
	operatorOutputMap.clear();
	operatorTypeResolverMap.clear();
	operators.insert("*");
	operatorOutputMap["*"] = "mul";
	operatorTypeResolverMap["*"] = MultiplicativeExpTypeResolver;
	operators.insert("/");
	operatorOutputMap["/"] = "div";
	operatorTypeResolverMap["/"] = MultiplicativeExpTypeResolver;
	operators.insert("%");
	operatorOutputMap["%"] = "rem";
	operatorTypeResolverMap["%"] = MultiplicativeExpTypeResolver;
	ParseLevelFunctor*			fifthLvl; 
	fifthLvl = new ParseLevelFunctor(operators, operatorOutputMap, operatorTypeResolverMap, fourthLvl);

	operators.clear();
	operatorOutputMap.clear();
	operatorTypeResolverMap.clear();
	operators.insert("+");
	operatorOutputMap["+"] = "add";
	operatorTypeResolverMap["+"] = AdditiveExpTypeResolver;
	operators.insert("-");
	operatorOutputMap["-"] = "sub";
	operatorTypeResolverMap["-"] = AdditiveExpTypeResolver;
	ParseLevelFunctor*			sixthLvl;
	sixthLvl = new ParseLevelFunctor(operators, operatorOutputMap, operatorTypeResolverMap, fifthLvl);

	operators.clear();
	operatorOutputMap.clear();
	operatorTypeResolverMap.clear();
	operators.insert("<");
	operatorOutputMap["<"] = "ltn";
	operatorTypeResolverMap["<"] = EqualityAndRelationalExpTypeResolver;
	operators.insert(">");
	operatorOutputMap[">"] = "gtn";
	operatorTypeResolverMap[">"] = EqualityAndRelationalExpTypeResolver;
	operators.insert("<=");
	operatorOutputMap["<="] = "leq";
	operatorTypeResolverMap["<="] = EqualityAndRelationalExpTypeResolver;
	operators.insert(">=");
	operatorOutputMap[">="] = "geq";
	operatorTypeResolverMap[">="] = EqualityAndRelationalExpTypeResolver;
	ParseLevelFunctor*			seventhLvl;
	seventhLvl = new ParseLevelFunctor(operators, operatorOutputMap, operatorTypeResolverMap, sixthLvl);

	operators.clear();
	operatorOutputMap.clear();
	operatorTypeResolverMap.clear();
	operators.insert("==");
	operatorOutputMap["=="] = "eql";
	operatorTypeResolverMap["=="] = EqualityAndRelationalExpTypeResolver;
	operators.insert("!=");
	operatorOutputMap["!="] = "neq";
	operatorTypeResolverMap["!="] = EqualityAndRelationalExpTypeResolver;
	ParseLevelFunctor*			eighthLvl;
	eighthLvl = new ParseLevelFunctor(operators, operatorOutputMap, operatorTypeResolverMap, seventhLvl);

	operators.clear();
	operatorOutputMap.clear();
	operatorTypeResolverMap.clear();
	operators.insert("&&");
	operatorOutputMap["&&"] = "and";
	operatorTypeResolverMap["&&"] = LogicalExpTypeResolver;
	ParseLevelFunctor*			ninthLvl;
	ninthLvl = new ParseLevelFunctor(operators, operatorOutputMap, operatorTypeResolverMap, eighthLvl);

	operators.clear();
	operatorOutputMap.clear();
	operatorTypeResolverMap.clear();
	operators.insert("||");
	operatorOutputMap["||"] = "or";
	operatorTypeResolverMap["||"] = LogicalExpTypeResolver;
	m_tenthLvl = new ParseLevelFunctor(operators, operatorOutputMap, operatorTypeResolverMap, ninthLvl);

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

void GetBinaryChildrenTypes(TreeNode<ASTNodeVal>* node, Type* childArr)
{
	const list<TreeNode<ASTNodeVal>*>& children = node->getChildList();

	assert(children.size() == 2);

	size_t childNum = 0;
	for (list<TreeNode<ASTNodeVal>*>::const_iterator i = children.begin(), i_end = children.end();
		i != i_end; ++i, ++childNum) {
			if (!GetSymbolType((*i)->val, &childArr[childNum])) {
				if (!GetVariantType((*i)->val, &childArr[childNum])) {
					childArr[childNum] = (*i)->val.variantTypeNode.type;
				}
			}
	}
}

void GetUnaryChildrenType(TreeNode<ASTNodeVal>* node, Type* childType)
{
	list<TreeNode<ASTNodeVal>*>::const_iterator child = node->getChildList().begin();

	assert(node->getChildList().size() == 1);

	if (!GetSymbolType((*child)->val, childType)) {
		if (!GetVariantType((*child)->val, childType)) {
			*childType = (*child)->val.variantTypeNode.type;
		}
	}
}

void AdditiveExpTypeResolver(TreeNode<ASTNodeVal>* node, const string& op)
{
	Type childTypes[2];
	GetBinaryChildrenTypes(node, childTypes);

	node->val.variantTypeNode.isLvalue = false;

	if (isPointerType(childTypes[1])) {
		const string& opStr = node->val.variantTypeNode.variant.getStrVal();
		if (opStr == "add" && isNumericType(childTypes[0])) {
			node->val.variantTypeNode.type = childTypes[1];
		} else if (opStr == "sub" && isPointerType(childTypes[0])) {
			node->val.variantTypeNode.type.spec = Type::LONGINT;
		} else {
			outputError(node->val.lineNumber, "invalid operands to binary " + op);
		}
	} else if (isNumericType(childTypes[0]) && isNumericType(childTypes[1])) {
		if (childTypes[0].spec == Type::LONGINT || childTypes[1].spec == Type::LONGINT) {
			node->val.variantTypeNode.type.spec = Type::LONGINT;
		} else {
			node->val.variantTypeNode.type.spec = Type::INT;
		}
	} else if (isPointerType(childTypes[0]) && isNumericType(childTypes[1])) {
		node->val.variantTypeNode.type = childTypes[0];
	} else {
		outputError(node->val.lineNumber, "invalid operands to binary " + op);
	}

	promoteType(node->val.variantTypeNode.type);
}

void MultiplicativeExpTypeResolver(TreeNode<ASTNodeVal>* node, const string& op)
{
	Type childTypes[2];
	GetBinaryChildrenTypes(node, childTypes);

	if (isNumericType(childTypes[0]) && isNumericType(childTypes[1])) {
		node->val.variantTypeNode.isLvalue = false;
		if (childTypes[0].spec == Type::LONGINT || childTypes[1].spec == Type::LONGINT) {
			node->val.variantTypeNode.type.spec = Type::LONGINT;
		} else {
			node->val.variantTypeNode.type.spec = Type::INT;
		}
	} else {
		outputError(node->val.lineNumber, "invalid operands to binary " + op);
	}
}

void LogicalExpTypeResolver(TreeNode<ASTNodeVal>* node, const string& op)
{
	Type childTypes[2];
	GetBinaryChildrenTypes(node, childTypes);

	if (!isLogicalType(childTypes[0]) || !isLogicalType(childTypes[1])) {
		outputError(node->val.lineNumber, "invalid operands to binary " + op);
	}

	node->val.variantTypeNode.isLvalue = false;
	node->val.variantTypeNode.type.spec = Type::INT;
}

void EqualityAndRelationalExpTypeResolver(TreeNode<ASTNodeVal>* node, const string& op)
{
	Type childTypes[2];
	GetBinaryChildrenTypes(node, childTypes);

	if (!typesCompatible(childTypes[0], childTypes[1])) {
		outputError(node->val.lineNumber, "invalid operands to binary " + op);
	}

	node->val.variantTypeNode.isLvalue = false;
	node->val.variantTypeNode.type.spec = Type::INT;
}

void DerefTypeResolver(TreeNode<ASTNodeVal>* node, const string& op)
{
	Type childType;
	GetUnaryChildrenType(node, &childType);

	if (!isPointerType(childType)) {
		outputError(node->val.lineNumber, "invalid operand to unary " + op);
	}

	node->val.variantTypeNode.isLvalue = true;
	node->val.variantTypeNode.type = childType;

	if (node->val.variantTypeNode.type.lvlsOfIndirection > 0) {
		node->val.variantTypeNode.type.lvlsOfIndirection--;
	}
}

void AddrOfTypeResolver(TreeNode<ASTNodeVal>* node, const string& op)
{
	const TreeNode<ASTNodeVal>* child = *node->getChildList().begin();
	bool symbolLvalue = child->val.type == ASTNodeValType::SYMBOL &&
						child->val.symbol->second.type.arraySize <= 0 &&
						!child->val.symbol->second.type.isFunction;
	bool variantTypeNodeLvalue = child->val.type == ASTNodeValType::VARIANTTYPENODE &&
								child->val.variantTypeNode.isLvalue;

	// symbols are lvalues
	if (!symbolLvalue && !variantTypeNodeLvalue) {
		outputError(node->val.lineNumber, "lvalue required in expression");
	}

	node->val.variantTypeNode.isLvalue = false;
	
	GetUnaryChildrenType(node, &node->val.variantTypeNode.type);

	node->val.variantTypeNode.type.lvlsOfIndirection++;
}

void NotTypeResolver(TreeNode<ASTNodeVal>* node, const string& op)
{
	Type childType;
	GetUnaryChildrenType(node, &childType);

	// symbols are lvalues
	if (!isLogicalType(childType)) {
		outputError(node->val.lineNumber, "invalid operand to unary " + op);
	}

	node->val.variantTypeNode.isLvalue = false;
	node->val.variantTypeNode.type.spec = Type::INT;
}

void NegTypeResolver(TreeNode<ASTNodeVal>* node, const string& op)
{
	Type childType;
	GetUnaryChildrenType(node, &childType);

	// symbols are lvalues
	if (!isNumericType(childType)) {
		outputError(node->val.lineNumber, "invalid operand to unary " + op);
	}

	node->val.variantTypeNode.isLvalue = false;
	node->val.variantTypeNode.type = childType;
}

void SizeOfTypeResolver(TreeNode<ASTNodeVal>* node, const string& op)
{
	Type childType;
	GetUnaryChildrenType(node, &childType);

	// symbols are lvalues
	if (childType.isFunction) {
		outputError(node->val.lineNumber, "invalid operand in sizeof expression");
	}

	node->val.variantTypeNode.isLvalue = false;
	node->val.variantTypeNode.type.spec = Type::LONGINT;
}

void IndexTypeResolver(TreeNode<ASTNodeVal>* node, const string& op)
{
	Type childTypes[2];
	GetBinaryChildrenTypes(node, childTypes);
	
	if (!isPointerType(childTypes[0]) || !isNumericType(childTypes[1])) {
		outputError(node->val.lineNumber, "invalid operands to binary " + op);
	}

	node->val.variantTypeNode.isLvalue = true;
	node->val.variantTypeNode.type = childTypes[0];
	node->val.variantTypeNode.type.arraySize = 0;

	if (node->val.variantTypeNode.type.lvlsOfIndirection > 0) {
		node->val.variantTypeNode.type.lvlsOfIndirection--;
	}
}