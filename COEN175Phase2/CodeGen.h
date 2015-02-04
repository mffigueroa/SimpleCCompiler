#ifndef __CODEGEN_H__
#define __CODEGEN_H__

#include <vector>

#include "Header.h"

typedef std::vector<SymbolTableRef> RegisterAllocation;

void CodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc);
void FuncCodeGen(TreeNode<ASTNodeVal>* node);
void BlockCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc);
void StatementCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc);

void ReturnCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc);
void WhileCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc);
void IfCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc);
void AssignCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc);
void ExpressionCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc);

#endif