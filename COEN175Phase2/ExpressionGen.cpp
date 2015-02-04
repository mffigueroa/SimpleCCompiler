#include <string>

using namespace std;

#include "CodeGen.h"

void MultiplyCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc);
void DivideCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc);
void AddCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc);
void SubtractCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc);

void ExpressionCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc)
{
	const string& nodeName = node->val.variant.getStrVal();
	if (nodeName == "mul") {
		MultiplyCodeGen(node, regAlloc);
	} else if (nodeName == "div") {
		DivideCodeGen(node, regAlloc);
	} else if (nodeName == "add") {
		AddCodeGen(node, regAlloc);
	} else if (nodeName == "sub") {
		SubtractCodeGen(node, regAlloc);
	}
}

void MultiplyCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc)
{
	// evaluate the two child nodes
	list<TreeNode<ASTNodeVal>*>::const_iterator child = node->getChildList().begin();
	CodeGen(*child, regAlloc);
	++child;
	CodeGen(*child, regAlloc);

	;
}

void DivideCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc)
{
	// evaluate the two child nodes
	list<TreeNode<ASTNodeVal>*>::const_iterator child = node->getChildList().begin();
	CodeGen(*child, regAlloc);
	++child;
	CodeGen(*child, regAlloc);

	;
}


void AddCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc)
{
	// evaluate the two child nodes
	list<TreeNode<ASTNodeVal>*>::const_iterator child = node->getChildList().begin();
	CodeGen(*child, regAlloc);
	++child;
	CodeGen(*child, regAlloc);

	;
}


void SubtractCodeGen(TreeNode<ASTNodeVal>* node, RegisterAllocation& regAlloc)
{
	// evaluate the two child nodes
	list<TreeNode<ASTNodeVal>*>::const_iterator child = node->getChildList().begin();
	CodeGen(*child, regAlloc);
	++child;
	CodeGen(*child, regAlloc);

	;
}