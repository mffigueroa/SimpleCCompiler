#include <string>

using namespace std;

#include "CodeGen.h"

Type SymbolAccessCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, bool expectPointer);
Type MultiplyCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state);
Type DivideCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state);
Type AddCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state);
Type SubtractCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state);

void BinaryOpParamCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, Type& r_lhs, Type& r_rhs);

Type ExpressionCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, bool expectPointer)
{
	if (node->val.type == ASTNodeValType::SYMBOL) {
		return SymbolAccessCodeGen(ss, node, state, expectPointer);
	}

	const string& nodeName = node->val.variant.getStrVal();
	if (nodeName == "mul") {
		return MultiplyCodeGen(ss, node, state);
	} else if (nodeName == "div") {
		return DivideCodeGen(ss, node, state);
	} else if (nodeName == "add") {
		return AddCodeGen(ss, node, state);
	} else if (nodeName == "sub") {
		return SubtractCodeGen(ss, node, state);
	}
}

Type SymbolAccessCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, bool expectPointer)
{
	// puts a pointer to the symbol in %rax
	const Symbol& sym = node->val.symbol->second;
	Indent(ss);

	if (expectPointer) {
		ss << "lea";
	} else {
		ss << "mov";
	}

	ss << GetInstSuffixForType(sym.type) << " " << GetSymbolPointer(sym, state) << ", %rax" << endl;
	return sym.type;
}

// evaluates the two binary parameters and stores the lhs in %rax and the rhs in %rdi
void BinaryOpParamCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, Type& r_lhsType, Type& r_rhsType)
{
	// evaluate the two child nodes
	list<TreeNode<ASTNodeVal>*>::iterator child = node->getChildList().begin();
	TreeNode<ASTNodeVal>* lhs = *child;
	++child;
	TreeNode<ASTNodeVal>* rhs = *child;

	r_rhsType = ExpressionCodeGen(ss, rhs, state);
	size_t rhsTypeSize = GetTypeSize(r_rhsType);
	char rhsTypeInstSuffix = GetInstSuffixForType(r_rhsType);

	// push the lhs value on the stack while we evaluate the
	// next expression
	size_t scopeStackOffsetIndex = state.scopeStackOffsets.size() - 2;
	state.scopeStackOffsets[scopeStackOffsetIndex] += rhsTypeSize;

	Indent(ss);
	ss << "push" << rhsTypeInstSuffix << " " << GetRegNameForType(r_rhsType, Registers::RAX) << endl;

	r_lhsType = ExpressionCodeGen(ss, lhs, state);

	// pop the lhs value off the stack into %rdi
	state.scopeStackOffsets[scopeStackOffsetIndex] -= rhsTypeSize;
	state.regAlloc[Registers::RDI] = true;

	Indent(ss);
	ss << "pop" << rhsTypeInstSuffix << " " << GetRegNameForType(r_rhsType, Registers::RDI) << endl;
}

Type MultiplyCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state)
{
	Type lhsType, rhsType;
	BinaryOpParamCodeGen(ss, node, state, lhsType, rhsType);

	ss << "imul" << GetInstSuffixForType(lhsType) << " " << GetRegNameForType(rhsType, Registers::RDI) << endl;

	state.regAlloc[Registers::RAX] = true;

	return lhsType;
}

Type DivideCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state)
{
	// evaluate the two child nodes
	list<TreeNode<ASTNodeVal>*>::const_iterator child = node->getChildList().begin();
	Type lhsType = ExpressionCodeGen(ss, *child, state);
	++child;
	Type rhsType = ExpressionCodeGen(ss, *child, state);

	return rhsType;
}


Type AddCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state)
{
	Type lhsType, rhsType;
	BinaryOpParamCodeGen(ss, node, state, lhsType, rhsType);

	ss << "add" << GetInstSuffixForType(lhsType) << " " << GetRegNameForType(rhsType, Registers::RDI)
		<< ", " << GetRegNameForType(lhsType, Registers::RAX) << endl;
	state.regAlloc[Registers::RAX] = true;

	return rhsType;
}


Type SubtractCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state)
{
	Type lhsType, rhsType;
	BinaryOpParamCodeGen(ss, node, state, lhsType, rhsType);

	ss << "sub" << GetInstSuffixForType(lhsType) << " " << GetRegNameForType(rhsType, Registers::RDI)
		<< ", " << GetRegNameForType(lhsType, Registers::RAX) << endl;
	state.regAlloc[Registers::RAX] = true;

	return rhsType;
}