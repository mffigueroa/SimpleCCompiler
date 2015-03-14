#include <string>

using namespace std;

#include "CodeGen.h"

Type MultiplyCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
Type DivideCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState, bool doModulus = false);
Type BinaryOpCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, const string& opInst, CodeGenState& state, StatementGenState& stmtState);
Type AndOrCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, bool genAnd, CodeGenState& state, StatementGenState& stmtState);
Type NotCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
Type NegCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
Type AddrOfCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
Type DerefCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
Type SizeOfCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
Type CastCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
Type IndexCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState);
Type ComparisonCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, const string& comparisonType, CodeGenState& state, StatementGenState& stmtState);

void PromoteResultToType(stringstream& ss, const Type& fromType, const Type& toType, Registers::Reg reg = Registers::RAX);
void PromoteBinaryOpsToLarger(stringstream& ss, const Type& lhsType, Registers::Reg lhsReg, const Type& rhsType, Registers::Reg rhsReg, Type& r_largerType);

void BinaryOpParamCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, Type& r_lhs, Type& r_rhs, StatementGenState& stmtState, bool needLhsAddress = false, bool needRhsAddress = false);

Type ExpressionCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	if (node->val.type == ASTNodeValType::SYMBOL) {
		return SymbolAccessCodeGen(ss, node, state, stmtState);
	}

	if (node->val.type == ASTNodeValType::VARIANT) {
		return VariantCodeGen(ss, node, state, stmtState);
	}

	const string& nodeName = node->val.variantTypeNode.variant.getStrVal();
	if (nodeName == "addr") {
		return AddrOfCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "deref") {
		return DerefCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "sizeof") {
		return SizeOfCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "CAST") {
		return CastCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "INDEX") {
		return IndexCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "FUNC_CALL") {
		return FuncCallCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "mul") {
		return MultiplyCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "div") {
		return DivideCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "rem") {
		return DivideCodeGen(ss, node, state, stmtState, true);
	} else if (nodeName == "add") {
		return BinaryOpCodeGen(ss, node, "add", state, stmtState);
	} else if (nodeName == "sub") {
		return BinaryOpCodeGen(ss, node, "sub", state, stmtState);
	} else if (nodeName == "and") {
		return AndOrCodeGen(ss, node, true, state, stmtState);
	} else if (nodeName == "or") {
		return AndOrCodeGen(ss, node, false, state, stmtState);
	} else if (nodeName == "not") {
		return NotCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "neg") {
		return NegCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "ltn") {
		// use the setl instruction
		return ComparisonCodeGen(ss, node, "l", state, stmtState);
	} else if (nodeName == "gtn") {
		// use the setg instruction
		return ComparisonCodeGen(ss, node, "g", state, stmtState);
	} else if (nodeName == "leq") {
		// use the setle instruction
		return ComparisonCodeGen(ss, node, "le", state, stmtState);
	} else if (nodeName == "geq") {
		// use the setge instruction
		return ComparisonCodeGen(ss, node, "ge", state, stmtState);
	} else if (nodeName == "eql") {
		// use the sete instruction
		return ComparisonCodeGen(ss, node, "e", state, stmtState);
	} else if (nodeName == "neq") {
		// use the setne instruction
		return ComparisonCodeGen(ss, node, "ne", state, stmtState);
	} else if (nodeName == "BLOCK") {
		BlockCodeGen(ss, node, state, stmtState);
		return Type();
	}
}

Type SymbolAccessCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	// puts a pointer to the symbol in %rax
	const Symbol& sym = node->val.symbol->second;
	Indent(ss);

	bool returningPointer = stmtState.expectsAddress.top() && !(stmtState.expectsValueIfPointerType.top() && sym.type.lvlsOfIndirection > 0);

	if (returningPointer) {
		ss << "leaq";
	} else {
		ss << "mov" << GetInstSuffixForType(sym.type);
	}

	ss << " " << GetSymbolPointer(node->val.symbol, state) << ", ";

	if (returningPointer) {
		ss << "%rax" << endl;
	} else {
		ss << GetRegNameForType(sym.type, Registers::RAX) << endl;
	}

	// have to promote char's
	if (!returningPointer && GetTypeSize(sym.type) == 1) {
		Type newIntType = sym.type;
		newIntType.spec = Type::INT;
		PromoteResultToType(ss, sym.type, newIntType);
		return newIntType;
	}

	return sym.type;
}

// evaluates the two binary parameters and stores the lhs in %rax and the rhs in %rdi
void BinaryOpParamCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, Type& r_lhsType, Type& r_rhsType, StatementGenState& stmtState, bool needLhsAddress, bool needRhsAddress)
{
	// evaluate the two child nodes
	list<TreeNode<ASTNodeVal>*>::iterator child = node->getChildList().begin();
	TreeNode<ASTNodeVal>* lhs = *child;
	++child;
	TreeNode<ASTNodeVal>* rhs = *child;

	if (needRhsAddress) {
		stmtState.expectsAddress.push(true);
	} else {
		stmtState.expectsAddress.push(false);
	}

	r_rhsType = ExpressionCodeGen(ss, rhs, state, stmtState);

	stmtState.expectsAddress.pop();

	// push the lhs value on the stack while we evaluate the
	// next expression
	state.temporaryStackOffset += 8;

	Indent(ss);
	ss << "pushq %rax" << endl;

	if (needLhsAddress) {
		stmtState.expectsAddress.push(true);
	} else {
		stmtState.expectsAddress.push(false);
	}

	r_lhsType = ExpressionCodeGen(ss, lhs, state, stmtState);

	stmtState.expectsAddress.pop();

	// pop the lhs value off the stack into %rdi
	state.temporaryStackOffset -= 8;
	state.regAlloc[Registers::RDI] = true;

	Indent(ss);
	ss << "popq %rdi" << endl;
}

Type BinaryOpCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, const string& opInst, CodeGenState& state, StatementGenState& stmtState)
{
	Type lhsType, rhsType;
	BinaryOpParamCodeGen(ss, node, state, lhsType, rhsType, stmtState);

	const Type& resultantType = node->val.variantTypeNode.type;

	Type largerType;
	PromoteBinaryOpsToLarger(ss, lhsType, Registers::RAX, rhsType, Registers::RDI, largerType);

	Indent(ss);
	ss << opInst << GetInstSuffixForType(largerType) << " " << GetRegNameForType(largerType, Registers::RDI)
		<< ", " << GetRegNameForType(largerType, Registers::RAX) << endl;
	state.regAlloc[Registers::RAX] = true;

	if (largerType != resultantType) {
		PromoteResultToType(ss, largerType, resultantType);
	}

	return resultantType;
}

Type MultiplyCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	Type lhsType, rhsType;
	BinaryOpParamCodeGen(ss, node, state, lhsType, rhsType, stmtState);

	const Type& resultantType = node->val.variantTypeNode.type;

	Type largerType;
	PromoteBinaryOpsToLarger(ss, lhsType, Registers::RAX, rhsType, Registers::RDI, largerType);
	
	Indent(ss);
	ss << "imul" << GetInstSuffixForType(largerType) << " " << GetRegNameForType(largerType, Registers::RDI) << endl;

	state.regAlloc[Registers::RAX] = true;

	if (largerType != resultantType) {
		PromoteResultToType(ss, largerType, resultantType);
	}

	return resultantType;
}

Type DivideCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState, bool doModulus)
{
	// evaluate the two child nodes
	list<TreeNode<ASTNodeVal>*>::iterator child = node->getChildList().begin();
	TreeNode<ASTNodeVal>* lhs = *child;
	++child;
	TreeNode<ASTNodeVal>* rhs = *child;

	Type rhsType = ExpressionCodeGen(ss, rhs, state, stmtState);

	// put rhs in %rdi
	Indent(ss);
	ss << "mov" << GetInstSuffixForType(rhsType) << " " << GetRegNameForType(rhsType, Registers::RAX)
		<< ", " << GetRegNameForType(rhsType, Registers::RDI) << endl;

	Type lhsType = ExpressionCodeGen(ss, lhs, state, stmtState);

	Type largerType;
	PromoteBinaryOpsToLarger(ss, lhsType, Registers::RAX, rhsType, Registers::RDI, largerType);

	size_t largerTypeSize = GetTypeSize(largerType);

	/*

	cltd
	cqto

	movl %eax, %edx
	sarl $31, %edx

	*/
	
	if (largerTypeSize == 8) {
		Indent(ss);
		ss << "cqto" << endl;
	} else if (largerTypeSize == 4) {
		Indent(ss);
		ss << "cltd" << endl;
	}

	Indent(ss);
	ss << "idiv" << GetInstSuffixForType(largerType) << " " << GetRegNameForType(largerType, Registers::RDI) << endl;

	if (doModulus) {
		Indent(ss);
		ss << "mov" << GetInstSuffixForType(largerType) << " " << GetRegNameForType(largerType, Registers::RDX)
			<< ", " << GetRegNameForType(largerType, Registers::RAX) << endl;
	}

	Type resultantType = node->val.variantTypeNode.type;
	if (largerType != resultantType) {
		PromoteResultToType(ss, largerType, resultantType);
	}

	return resultantType;
}

Type AndOrCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, bool genAnd, CodeGenState& state, StatementGenState& stmtState)
{
	Type lhsType, rhsType;

	// evaluate the two child nodes
	list<TreeNode<ASTNodeVal>*>::iterator child = node->getChildList().begin();
	TreeNode<ASTNodeVal>* lhs = *child;
	++child;
	TreeNode<ASTNodeVal>* rhs = *child;

	const Type& resultantType = node->val.variantTypeNode.type;

	string lblPrefix = "_" + state.enclosingFunc->second.identifier + "_" + intToStr(stmtState.numAndOpsInFunc) + "_";

	if (genAnd) {
		lblPrefix.insert(0, "AND");
		++stmtState.numAndOpsInFunc;
	} else {
		lblPrefix.insert(0, "OR");
		++stmtState.numOrOpsInFunc;
	}

	stmtState.expectsAddress.push(false);
	lhsType = ExpressionCodeGen(ss, lhs, state, stmtState);

	string raxRegName = GetRegNameForType(lhsType, Registers::RAX);

	Indent(ss);
	ss << "test" << GetInstSuffixForType(lhsType) << " " << raxRegName << ", " << raxRegName << endl;

	Indent(ss);
	if (genAnd) {
		ss << "jz " << lblPrefix << "0" << endl;
	} else {
		ss << "jnz " << lblPrefix << "1" << endl;
	}

	rhsType = ExpressionCodeGen(ss, rhs, state, stmtState);
	stmtState.expectsAddress.pop();

	raxRegName = GetRegNameForType(rhsType, Registers::RAX);

	Indent(ss);
	ss << "test" << GetInstSuffixForType(lhsType) << " " << raxRegName << ", " << raxRegName << endl;

	Indent(ss);
	
	if (genAnd) {
		ss << "jz " << lblPrefix << "0" << endl;
	} else {
		ss << "jnz " << lblPrefix << "1" << endl;
	}

	raxRegName = GetRegNameForType(resultantType, Registers::RAX);

	Indent(ss);
	if (genAnd) {
		ss << "mov" << GetInstSuffixForType(resultantType) << " $1, " << raxRegName << endl;
	} else {
		ss << "mov" << GetInstSuffixForType(resultantType) << " $0, " << raxRegName << endl;
	}

	Indent(ss);
	ss << "jmp " << lblPrefix << "END" << endl;

	if (genAnd) {
		ss << lblPrefix << "0:" << endl;

		Indent(ss);
		ss << "mov" << GetInstSuffixForType(resultantType) << " $0, " << raxRegName << endl;
	} else {
		ss << lblPrefix << "1:" << endl;

		Indent(ss);
		ss << "mov" << GetInstSuffixForType(resultantType) << " $1, " << raxRegName << endl;
	}

	ss << lblPrefix << "END:" << endl;

	state.regAlloc[Registers::RAX] = true;

	return resultantType;
}

Type NegCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	list<TreeNode<ASTNodeVal>*>::const_iterator child = node->getChildList().begin();
	Type lhsType = ExpressionCodeGen(ss, *child, state, stmtState);
	Type resultantType = node->val.variantTypeNode.type;

	Indent(ss);
	ss << "neg" << GetInstSuffixForType(resultantType) << " " << GetRegNameForType(resultantType, Registers::RAX) << endl;
	state.regAlloc[Registers::RAX] = true;

	return resultantType;
}

Type NotCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	list<TreeNode<ASTNodeVal>*>::const_iterator child = node->getChildList().begin();
	Type lhsType = ExpressionCodeGen(ss, *child, state, stmtState);

	Type resultantType = node->val.variantTypeNode.type;

	Indent(ss);
	ss << "cmp" << GetInstSuffixForType(lhsType) << " $0, " << GetRegNameForType(lhsType, Registers::RAX) << endl;
	
	Indent(ss);
	ss << "setz %al" << endl;

	Indent(ss);
	ss << "movzb" << GetInstSuffixForType(resultantType) << " %al, " << GetRegNameForType(resultantType, Registers::RAX) << endl;
	state.regAlloc[Registers::RAX] = true;

	return resultantType;
}

Type ComparisonCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, const string& comparisonType, CodeGenState& state, StatementGenState& stmtState)
{
	Type lhsType, rhsType;
	BinaryOpParamCodeGen(ss, node, state, lhsType, rhsType, stmtState);

	Type resultantType = node->val.variantTypeNode.type;

	Indent(ss);
	ss << "cmp" << GetInstSuffixForType(lhsType) << " " << GetRegNameForType(rhsType, Registers::RDI)
		<< ", " << GetRegNameForType(lhsType, Registers::RAX) << endl;

	Indent(ss);
	ss << "set" << comparisonType << " %al" << endl;

	if (resultantType.spec != Type::CHAR) {
		Indent(ss);
		ss << "movzb" << GetInstSuffixForType(resultantType) << " %al, " << GetRegNameForType(resultantType, Registers::RAX) << endl;
	}

	state.regAlloc[Registers::RAX] = true;

	return resultantType;
}

Type AddrOfCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	list<TreeNode<ASTNodeVal>*>::const_iterator child = node->getChildList().begin();

	stmtState.expectsAddress.push(true);
	ExpressionCodeGen(ss, *child, state, stmtState);
	stmtState.expectsAddress.pop();

	return node->val.variantTypeNode.type;
}

Type DerefCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	list<TreeNode<ASTNodeVal>*>::const_iterator child = node->getChildList().begin();

	stmtState.expectsAddress.push(false);
	Type t = ExpressionCodeGen(ss, *child, state, stmtState);
	stmtState.expectsAddress.pop();
	
	Type resultantType = node->val.variantTypeNode.type;

	if (stmtState.expectsAddress.top()) {
		// we know that t will be a pointer so it'll be 8 bytes and in %rax
		Indent(ss);
		ss << "mov" << GetInstSuffixForType(resultantType) << " (%rax), " << GetRegNameForType(resultantType, Registers::RAX) << endl;
	}

	return resultantType;
}

Type SizeOfCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	list<TreeNode<ASTNodeVal>*>& children = node->getChildList();
	list<TreeNode<ASTNodeVal>*>::iterator child = children.begin();
	
	Type t;

	if (children.size() == 2) {
		t.spec = (*child)->val.variant.getSpecifierVal();
		++child;
		t.lvlsOfIndirection = (*child)->val.variant.getIntVal();
	} else {
		const ASTNodeVal& i = (*child)->val;

		if (i.type == ASTNodeValType::SYMBOL) {
			t = i.symbol->second.type;
		} else if (i.type == ASTNodeValType::VARIANTTYPENODE) {
			t = i.variantTypeNode.type;
		} else if (i.type == ASTNodeValType::VARIANT) {
			Variant::VariantType v = i.variant.getType();
			if (v == Variant::INT) {
				t.spec = Type::INT;
			} else if (v == Variant::LONGINT) {
				t.spec = Type::LONGINT;				
			} else if (v == Variant::CHAR) {
				t.spec = Type::CHAR;				
			} else if (v == Variant::STRING) {
				t.spec = Type::CHAR;
				// add one for null terminator
				t.arraySize = i.variant.getStrVal().length() + 1;
			}
		}
	}

	Indent(ss);
	ss << "movq $" << GetTypeSize(t) << ", %rax" << endl;

	return node->val.variantTypeNode.type;
}

Type IndexCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	Type lhsType, rhsType;

	stmtState.expectsValueIfPointerType.push(true);

	BinaryOpParamCodeGen(ss, node, state, lhsType, rhsType, stmtState, true, false);
	
	stmtState.expectsValueIfPointerType.pop();

	string lhsTypeSizeStr = intToStr(GetTypeSize(lhsType, false));

	if (GetTypeSize(rhsType) < 8) {
		Type t;
		t.spec = Type::LONGINT;
		PromoteResultToType(ss, rhsType, t, Registers::RDI);
	}

	Indent(ss);

	bool computeAddress = stmtState.expectsAddress.top() && !(stmtState.expectsValueIfPointerType.top() && isPointerType(lhsType));

	Type resultantType = node->val.variantTypeNode.type;
	
	if (computeAddress) {
		ss << "leaq";
	} else {
		ss << "mov" << GetInstSuffixForType(resultantType);
	}

	ss << " (%rax, %rdi, " << lhsTypeSizeStr << "), ";
	if (computeAddress) {
		ss << "%rax" << endl;
	} else {
		ss << GetRegNameForType(resultantType, Registers::RAX) << endl;
	}

	return resultantType;
}

Type CastCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	list<TreeNode<ASTNodeVal>*>::const_iterator child = node->getChildList().begin();
	++child;
	++child;
	return ExpressionCodeGen(ss, *child, state, stmtState);
}

void PromoteResultToType(stringstream& ss, const Type& fromType, const Type& toType, Registers::Reg reg)
{
	size_t srcTypeSize = GetTypeSize(fromType), destTypeSize = GetTypeSize(toType);
	char instSuffix = GetInstSuffixForType(toType);

	Indent(ss);
	ss << "movs" << GetInstSuffixForType(fromType) << GetInstSuffixForType(toType) << " " << GetRegNameForType(fromType, reg) << ", " << GetRegNameForType(toType, reg) << endl;
}

void PromoteBinaryOpsToLarger(stringstream& ss, const Type& lhsType, Registers::Reg lhsReg, const Type& rhsType, Registers::Reg rhsReg, Type& r_largerType)
{
	// if the operands aren't the same size, make the smaller one bigger
	size_t lhsTypeSize = GetTypeSize(lhsType), rhsTypeSize = GetTypeSize(rhsType);

	r_largerType = lhsType;

	if (lhsTypeSize < rhsTypeSize) {
		r_largerType = rhsType;
		PromoteResultToType(ss, lhsType, rhsType, lhsReg);
	} else if (lhsTypeSize > rhsTypeSize) {
		PromoteResultToType(ss, rhsType, lhsType, rhsReg);
	}
}