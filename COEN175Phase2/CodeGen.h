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

	extern std::map<size_t, std::string> regNames;
	extern std::set<size_t> callerSaved, calleeSaved;
}

typedef struct _CodeGenState {
	bool				regAlloc[Registers::NUM_REG];
	std::vector<size_t>	scopeStackOffsets;
	std::string			enclosingFuncName;
	std::stack<size_t>	numWhileStatementsInBlock;
	std::stack<size_t>	numIfStatementsInBlock;

	_CodeGenState();
} CodeGenState;

void		InitRegMaps();
void		Indent(std::stringstream& ss);
std::string GetRegNameForType(const Type& t, Registers::Reg reg);
char		GetInstSuffixForType(const Type& t);
std::string	GetSymbolPointer(const Symbol& sym, CodeGenState& state);

void RootCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node);

void GlobalVarGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node);

size_t AllocateStackForBlock(TreeNode<ASTNodeVal>* node);
void FuncCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node);

void BlockCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state);
void StatementCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state);

void ReturnCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state);
void WhileCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state);
void IfCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state);
void AssignCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state);

Type ExpressionCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, bool expectPointer = false);

#endif