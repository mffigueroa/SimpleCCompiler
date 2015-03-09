#ifndef __HEADER_H__
#define __HEADER_H__

#include <list>
#include <map>
#include <vector>
#include <set>
#include <string>

#include "Type.h"
#include "Symbol.h"
#include "Tree.h"
#include "Variant.h"

typedef std::multimap<std::string, Symbol>::iterator SymbolTableRef;

typedef struct _VariantTypeNode
{
	Variant variant;
	Type	type;
	bool	isLvalue;

	_VariantTypeNode() :
		isLvalue(false) {}
} VariantTypeNode;

namespace ASTNodeValType
{
	typedef enum
	{
		SYMBOL,
		VARIANT,
		VARIANTTYPENODE
	} eASTNodeValType;
};

typedef struct _ASTNodeVal {
	SymbolTableRef					symbol;
	Variant							variant;
	VariantTypeNode					variantTypeNode;
	unsigned int					lineNumber;
	ASTNodeValType::eASTNodeValType	type;

	_ASTNodeVal();
} ASTNodeVal;

typedef std::map<std::string, SymbolTableRef>	Scope;
typedef std::list<Scope>						ScopeStack;

typedef struct
{
	ScopeStack							stack;
	std::multimap<std::string, Symbol>	symbolTable;
} ParserState;

extern std::vector<std::string>	operatorNames;

void match(const std::string& tokenType, Variant* v = 0, unsigned int* lineNumber = 0);
std::string lookahead(unsigned int ahead = 0);

bool		isSpecifier(const std::string& str);
bool		LookupSymbol(const ScopeStack& stack, const std::string& symbolName, SymbolTableRef* r_symbol = 0);
void		SkipErrorUntil(const std::string& synchronizeToken);
void		outputError(unsigned int lineNumber, const std::string& err);
void		outputDotFile(TreeNode<ASTNodeVal>* v, const std::string& filename);
void		renderDotFile(const std::string& filename);
void		FreeNodeList(std::list<TreeNode<ASTNodeVal>*>& children);
bool		addChildrenToNode(TreeNode<ASTNodeVal>* v, const std::list<TreeNode<ASTNodeVal>*>& children);
bool		cmpFuncWithoutParams(const Symbol& lhs, const Symbol& rhs);
std::string intToStr(int i);
bool		GetSymbolType(const ASTNodeVal& node, Type* t, bool* isLvalue = NULL);
bool		GetVariantType(const ASTNodeVal& node, Type* t);

void TranslationUnit(TreeNode<ASTNodeVal>** r_astRootNode, ParserState** r_parserState);

size_t Pointers();
Type::eSpecifier Specifier();
int	 Number();

TreeNode<ASTNodeVal>*				FunctionDefinition(ParserState& parserState);
TreeNode<ASTNodeVal>*				Parameters(ParserState& parserState, std::vector<Symbol*>& parameters);
TreeNode<ASTNodeVal>*				ParameterList(ParserState& parserState);
TreeNode<ASTNodeVal>*				Parameter(ParserState& parserState, Symbol& s);
TreeNode<ASTNodeVal>*				Declarations(ParserState& parserState);
std::list<TreeNode<ASTNodeVal>*>	Declaration(ParserState& parserState, bool global = false);
TreeNode<ASTNodeVal>*				Declarator(ParserState& parserState, Type::eSpecifier spec, bool global = false);
TreeNode<ASTNodeVal>*				Statements(ParserState& parserState, Type* enclosingFuncRetType);
TreeNode<ASTNodeVal>*				Statement(ParserState& parserState, Type* enclosingFuncRetType);
std::list<TreeNode<ASTNodeVal>*>	ExpressionList(ParserState& parserState);
TreeNode<ASTNodeVal>*				Expression(ParserState& parserState);

#endif