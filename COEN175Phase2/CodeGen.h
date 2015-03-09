#ifndef __CODEGEN_H__
#define __CODEGEN_H__

#include <vector>
#include <map>
#include <string>
#include <set>
#include <stack>
#include <sstream>

#include "Header.h"

namespace Registers
{
	enum Reg
	{
		RAX = 0, RBX, RCX, RDX, RSI, RDI, R8, R9, R10, R11, R12, R13, R14, R15, NUM_REG
	};

	extern std::vector<size_t>				parameterRegisters, calleeSaved;
	extern std::map<size_t, std::string>	regNames;
	extern std::set<size_t>					callerSaved;
}

extern std::map<std::string, std::string> constStrGlobals;

typedef struct _CodeGenState {
	bool										regAlloc[Registers::NUM_REG];
	std::vector<size_t>							scopeStackAllocSizes;
	size_t										temporaryStackOffset;
	SymbolTableRef								enclosingFunc;
	size_t										numWhileStatementsInFunc;
	size_t										numIfStatementsInFunc;
	std::vector<std::vector<SymbolTableRef>>	scopeVariables;
	std::map<Symbol*, size_t>					funcParameters;

	_CodeGenState();
} CodeGenState;

typedef struct _StatementGenState {
	std::stack<bool> expectsAddress;
	std::stack<bool> expectsValueIfPointerType;

	_StatementGenState();
} StatementGenState;

void		InitRegMaps();
void		Indent(std::stringstream& ss);
std::string GetRegNameForType(const Type& t, Registers::Reg reg);
char		GetInstSuffixForType(const Type& t);
std::string	GetSymbolPointer(const SymbolTableRef symRef, CodeGenState& state);

void RootCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node);

void GlobalVarGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node);

size_t AllocateStackForBlock(TreeNode<ASTNodeVal>* node, CodeGenState& state);
void FuncCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node);

void BlockCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
void StatementCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);

Type FuncCallCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
void ReturnCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
void WhileCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
void IfCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
void AssignCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);

Type ExpressionCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
Type SymbolAccessCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
Type VariantCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
void StringVariantCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node);

#endif