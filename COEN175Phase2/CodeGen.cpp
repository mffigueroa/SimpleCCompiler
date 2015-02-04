#include <list>
#include <algorithm>

using namespace std;

#include "CodeGen.h"

void CodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc)
{
	if (node->val.isSymbol) {
		if (node->val.symbol->second.type.isFunction) {
			FuncCodeGen(node);
		} else {
			RegisterAllocation::iterator sym = find(regAlloc.begin(), regAlloc.end(), node->val.symbol);
		}
	} else {
		const string& nodeName = node->val.variant.getStrVal();

		if (nodeName == "TranslationUnit") {
			const list<TreeNode<ASTNodeVal>*>& children = node->getChildList();
			list<TreeNode<ASTNodeVal>*>::const_iterator i = children.begin(), i_end = children.end();
			for (; i != i_end; ++i) {
				CodeGen(*i, regAlloc);
			}
		}
	}
}

void FuncCodeGen(TreeNode<ASTNodeVal>* node)
{
	RegisterAllocation regAlloc;
	BlockCodeGen(node, regAlloc);
}

void BlockCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc)
{
	const list<TreeNode<ASTNodeVal>*>& children = node->getChildList();
	list<TreeNode<ASTNodeVal>*>::const_iterator it = children.begin(), it_end = children.end();
	for (; it != it_end; ++it) {
		if (!(*it)->val.isSymbol && (*it)->val.variant.getStrVal() == "DECLS") {
			break;
		}
	}

	if (it != it_end) {
		const list<TreeNode<ASTNodeVal>*>& declChildren = (*it)->getChildList();
		list<TreeNode<ASTNodeVal>*>::const_iterator declIt = declChildren.begin(), declIt_end = declChildren.end();
		for (; declIt != declIt_end; ++declIt) {
			if ((*declIt)->val.isSymbol) {
				regAlloc.push_back((*declIt)->val.symbol);
			}
		}
	}

	it = children.begin(), it_end = children.end();
	for (; it != it_end; ++it) {
		if (!(*it)->val.isSymbol && (*it)->val.variant.getStrVal() == "STMTS") {
			const list<TreeNode<ASTNodeVal>*>& stmtsChildren = (*it)->getChildList();
			list<TreeNode<ASTNodeVal>*>::const_iterator stmtIt = stmtsChildren.begin(), stmtIt_end = stmtsChildren.end();
			
			for (; stmtIt != stmtIt_end; ++stmtIt) {
				StatementCodeGen(*stmtIt, regAlloc);
			}
		}
	}
}

void StatementCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc)
{
	const string& nodeName = node->val.variant.getStrVal();

	if (nodeName == "BLOCK") {
		BlockCodeGen(node, regAlloc);
	} else if (nodeName == "RETURN") {
		ReturnCodeGen(node, regAlloc);
	} else if (nodeName == "WHILE") {
		WhileCodeGen(node, regAlloc);
	} else if (nodeName == "IF") {
		IfCodeGen(node, regAlloc);
	} else if (nodeName == "ASSIGN") {
		AssignCodeGen(node, regAlloc);
	} else if (find(operatorNames.begin(), operatorNames.end(), nodeName) != operatorNames.end()) {
		ExpressionCodeGen(node, regAlloc);
	}
}

void ReturnCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc)
{
	;
}

void WhileCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc)
{
	;
}

void IfCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc)
{
	;
}

void AssignCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc)
{
	;
}