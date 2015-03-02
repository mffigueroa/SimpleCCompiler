#include <iostream>
#include <list>
#include <algorithm>

using namespace std;

#include "CodeGen.h"

_CodeGenState::_CodeGenState()
{
	for (size_t i = 0; i < Registers::NUM_REG; ++i) {
		regAlloc[i] = false;
	}
}

namespace Registers
{
	map<size_t, string> regNames;
	set<size_t> callerSaved, calleeSaved;
}

void InitRegMaps()
{
	Registers::regNames[Registers::RAX] = "%rax";
	Registers::regNames[Registers::RBX] = "%rbx";
	Registers::regNames[Registers::RCX] = "%rcx";
	Registers::regNames[Registers::RDX] = "%rdi";
	Registers::regNames[Registers::RSI] = "%rsi";
	Registers::regNames[Registers::RDI] = "%rdi";
	Registers::regNames[Registers::R8] = "%r8";
	Registers::regNames[Registers::R9] = "%r9";
	Registers::regNames[Registers::R10] = "%r10";
	Registers::regNames[Registers::R11] = "%r11";
	Registers::regNames[Registers::R12] = "%r12";
	Registers::regNames[Registers::R13] = "%r13";
	Registers::regNames[Registers::R14] = "%r14";
	Registers::regNames[Registers::R15] = "%r15";

	Registers::calleeSaved.insert(Registers::RBX);
	Registers::calleeSaved.insert(Registers::R12);
	Registers::calleeSaved.insert(Registers::R13);
	Registers::calleeSaved.insert(Registers::R14);
	Registers::calleeSaved.insert(Registers::R15);

	for (size_t i = 0; i < Registers::NUM_REG; ++i) {
		if (Registers::calleeSaved.find(i) == Registers::calleeSaved.end()) {
			Registers::callerSaved.insert(i);
		}
	}
}

string GetRegNameForType(const Type& t, Registers::Reg reg)
{
	string origName = Registers::regNames[reg];
	size_t typeSize = GetTypeSize(t);

	// NOT SURE IF THIS MAKES SENSE.
	// WHEN WOULD THIS FUNCTION BE CALLED
	// WITH AN ARRAY AS A PARAMETER
	if (t.arraySize > 0) {
		typeSize /= t.arraySize;
	}

	if ('0' <= origName[1] && origName[1] <= '9') {
		if (typeSize == 4) {
			origName += 'd';
		}

		if (typeSize == 1) {
			origName += 'b';
		}

		return origName;
	} else {
		if (typeSize == 4) {
			origName[1] = 'e';
		}

		if (typeSize == 1) {
			origName[1] = origName[2];
			origName[2] = 'l';
			return origName.substr(0, 3);
		}

		return origName;
	}
}

char GetInstSuffixForType(const Type& t)
{
	size_t typeSize = GetTypeSize(t);

	if (typeSize == 8) {
		return 'q';
	}

	if (typeSize == 4) {
		return 'l';
	}

	if (typeSize == 1) {
		return 'b';
	}
}

void Indent(stringstream& ss)
{
	ss << "\t\t";
}

void RootCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node)
{
	if (node->val.type == ASTNodeValType::SYMBOL) {
		// if this is a function declaration, ignore it. but if its a definition
		// then generate its code.
		if (node->val.symbol->second.type.isFunction && node->getChildList().size() > 0) {
			FuncCodeGen(ss, node);
		} else {
			// declare the global variables in the assembly code
			GlobalVarGen(ss, node);
		}
	} else if (node->val.type == ASTNodeValType::VARIANT) {
		const string& nodeName = node->val.variant.getStrVal();

		if (nodeName == "TranslationUnit") {
			// start the assembly file, declaring this to be
			// the .text section
			ss << ".text" << endl;

			const list<TreeNode<ASTNodeVal>*>& children = node->getChildList();
			list<TreeNode<ASTNodeVal>*>::const_iterator i = children.begin(), i_end = children.end();
			for (; i != i_end; ++i) {
				// generate code for every definition of
				// a function or a global variable
				RootCodeGen(ss, *i);
			}
		}
	}
}

void GlobalVarGen(stringstream& ss, TreeNode<ASTNodeVal>* node)
{
	const Symbol& sym = node->val.symbol->second;

	ss << sym.identifier << ":" << endl;

	size_t varSize = 0;

	if (sym.type.lvlsOfIndirection > 0) {
		// a pointer
		varSize = 8;
	} else if (sym.type.spec == Type::LONGINT) {
		varSize = 8;
	} else if (sym.type.spec == Type::INT) {
		varSize = 4;
	} else if (sym.type.spec == Type::CHAR) {
		varSize = 1;
	}

	Indent(ss);

	if (varSize == 8) {
		ss << ".quad ";
	} else if (varSize == 4) {
		ss << ".long ";
	} else if (varSize == 1) {
		ss << ".byte ";
	}

	for (size_t i = 0; i <= sym.type.arraySize; ++i) {
		if (i > 0) {
			ss << ",";
		}

		ss << "0";
	}
}

void FuncCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node)
{
	const Symbol& sym = node->val.symbol->second;

	ss << ".global " << sym.identifier << endl;
	ss << sym.identifier << ":" << endl;
	
	Indent(ss);
	ss << "pushq %rbp" << endl;
	Indent(ss);
	ss << "movq %rsp, %rbp" << endl;

	stringstream blockCode;
	CodeGenState blockState;

	blockState.enclosingFuncName = sym.identifier;
	blockState.numWhileStatementsInBlock.push(0);
	blockState.numIfStatementsInBlock.push(0);
	blockState.scopeStackOffsets.push_back(0);

	// the codegenstate passed in here will record
	// all the registers its used so we only have to
	// save them
	BlockCodeGen(blockCode, node, blockState);

	// save the callee saved registers to the stack
	for (size_t i = 0; i < Registers::NUM_REG; ++i) {
		if (blockState.regAlloc[i] && Registers::calleeSaved.find(i) != Registers::calleeSaved.end()) {
			Indent(ss);
			ss << "pushq " << Registers::regNames[i] << endl;
		}
	}

	ss << blockCode.str();

	// pop the callee saved registers from the stack in reverse order
	for (int i = Registers::NUM_REG - 1; i >= 0; --i) {
		if (blockState.regAlloc[i] && Registers::calleeSaved.find(i) != Registers::calleeSaved.end()) {
			Indent(ss);
			ss << "popq " << Registers::regNames[i] << endl;
		}
	}

	Indent(ss);
	ss << "movq %rbp, %rsp" << endl;
	Indent(ss);
	ss << "popq %rbp" << endl;
	Indent(ss);
	ss << "ret" << endl;
}

size_t AllocateStackForBlock(TreeNode<ASTNodeVal>* node)
{
	const list<TreeNode<ASTNodeVal>*>& children = node->getChildList();
	list<TreeNode<ASTNodeVal>*>::const_iterator declNode = children.begin();

	for (; declNode != children.end(); ++declNode) {
		if ((*declNode)->val.type == ASTNodeValType::VARIANT &&
			(*declNode)->val.variant.getStrVal() == "DECLS") {
				break;
		}
	}

	if (declNode == children.end()) {
		return 0;
	}

	const list<TreeNode<ASTNodeVal>*>& decls = node->getChildList();
	list<TreeNode<ASTNodeVal>*>::const_iterator currDecl = decls.begin();

	size_t totalDeclsSize = 0;

	for (; currDecl != decls.end(); ++currDecl) {
		if ((*currDecl)->val.type == ASTNodeValType::SYMBOL) {
			(*currDecl)->val.symbol->second.stackOffset = totalDeclsSize;
			totalDeclsSize += GetTypeSize((*currDecl)->val.symbol->second.type);
		}
	}

	return totalDeclsSize;
}

void BlockCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state)
{
	size_t stackSize = AllocateStackForBlock(node);

	// in accessing variables in this block we have to add the second to last stack offset
	// because we have to add the back all the subtractions to rsp of all but the current scope
	state.scopeStackOffsets.push_back(*state.scopeStackOffsets.rbegin() + stackSize);

	state.numIfStatementsInBlock.push(0);
	state.numWhileStatementsInBlock.push(0);

	Indent(ss);
	ss << "subq $" << stackSize << ", %rsp" << endl;
	
	const list<TreeNode<ASTNodeVal>*>& children = node->getChildList();
	list<TreeNode<ASTNodeVal>*>::const_iterator it = children.begin();

	for (; it != children.end(); ++it) {
		if ((*it)->val.type == ASTNodeValType::VARIANT && (*it)->val.variant.getStrVal() == "STMTS") {
			const list<TreeNode<ASTNodeVal>*>& stmtsChildren = (*it)->getChildList();
			list<TreeNode<ASTNodeVal>*>::const_iterator stmtIt = stmtsChildren.begin(), stmtIt_end = stmtsChildren.end();
			
			for (; stmtIt != stmtIt_end; ++stmtIt) {
				StatementCodeGen(ss, *stmtIt, state);
			}
		}
	}

	Indent(ss);
	ss << "addq $" << stackSize << ", %rsp" << endl;

	// get rid of our stack offset as we finish
	// generating code for this block
	state.scopeStackOffsets.pop_back();
	state.numIfStatementsInBlock.pop();
	state.numWhileStatementsInBlock.pop();
}

void StatementCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state)
{
	const string& nodeName = node->val.variant.getStrVal();

	if (nodeName == "BLOCK") {
		BlockCodeGen(ss, node, state);
	} else if (nodeName == "RETURN") {
		ReturnCodeGen(ss, node, state);
	} else if (nodeName == "WHILE") {
		WhileCodeGen(ss, node, state);
	} else if (nodeName == "IF") {
		IfCodeGen(ss, node, state);
	} else if (nodeName == "ASSIGN") {
		AssignCodeGen(ss, node, state);
	} else if (find(operatorNames.begin(), operatorNames.end(), nodeName) != operatorNames.end()) {
		ExpressionCodeGen(ss, node, state);
	}
}

void ReturnCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state)
{
	ExpressionCodeGen(ss, *node->getChildList().begin(), state);
}

void WhileCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state)
{
	list<TreeNode<ASTNodeVal>*>::iterator i = node->getChildList().begin();
	TreeNode<ASTNodeVal>* conditionNode = *i;
	++i;
	TreeNode<ASTNodeVal>* exprNode = *i;
	
	string lblPrefix = ".WHILE_" + state.enclosingFuncName + "_" + intToStr(state.numWhileStatementsInBlock.top()) + "_";

	Indent(ss);
	ss << lblPrefix << "START:" << endl;

	Type t = ExpressionCodeGen(ss, conditionNode, state);

	Indent(ss);
	string raxName = GetRegNameForType(t, Registers::RAX);
	ss << "test" << GetInstSuffixForType(t) << " " << raxName << ", " << raxName << endl;

	Indent(ss);
	ss << "jz " << lblPrefix << "END" << endl;

	ExpressionCodeGen(ss, exprNode, state);

	Indent(ss);
	ss << lblPrefix << "END:" << endl;

	state.numWhileStatementsInBlock.top()++;
}

void IfCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state)
{
	list<TreeNode<ASTNodeVal>*> children = node->getChildList();
	bool hasElseNode = children.size() == 3;

	list<TreeNode<ASTNodeVal>*>::iterator i = children.begin();
	TreeNode<ASTNodeVal>* conditionNode = *i;
	++i;
	TreeNode<ASTNodeVal>* stmtNode = *i;

	Indent(ss);
	Type t = ExpressionCodeGen(ss, conditionNode, state);

	string lblPrefix = ".IF_" + state.enclosingFuncName + "_" + intToStr(state.numIfStatementsInBlock.size()) + "_";

	Indent(ss);
	string raxName = GetRegNameForType(t, Registers::RAX);
	ss << "test" << GetInstSuffixForType(t) << " " << raxName << ", " << raxName << endl;

	Indent(ss);
	ss << "jnz " << lblPrefix << "IF" << endl;

	Indent(ss);

	if (hasElseNode) {
		ss << "jmp " << lblPrefix << "ELSE" << endl;
	} else {
		ss << "jmp " << lblPrefix << "END" << endl;
	}

	Indent(ss);
	ss << lblPrefix << "IF:" << endl;

	StatementCodeGen(ss, stmtNode, state);

	Indent(ss);
	ss << "jmp " << lblPrefix << "END" << endl;

	if (hasElseNode) {
		Indent(ss);
		ss << lblPrefix << "ELSE:" << endl;

		++i;
		TreeNode<ASTNodeVal>* elseNode = *i;
		StatementCodeGen(ss, *elseNode->getChildList().begin(), state);
	}

	Indent(ss);
	ss << lblPrefix << "END:" << endl;

	state.numIfStatementsInBlock.top()++;
}

void AssignCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state)
{
	list<TreeNode<ASTNodeVal>*>::iterator i = node->getChildList().begin();
	TreeNode<ASTNodeVal>* lhs = *i;
	++i;
	TreeNode<ASTNodeVal>* rhs = *i;

	// generate code to give the address of the lhs, given its
	// an l-value
	Type lhsType = ExpressionCodeGen(ss, lhs, state, true);
	size_t lhsTypeSize = GetTypeSize(lhsType);
	char lhsTypeInstSuffix = GetInstSuffixForType(lhsType);

	// push the lhs value on the stack while we evaluate the
	// next expression
	size_t scopeStackOffsetIndex = state.scopeStackOffsets.size() - 2;
	state.scopeStackOffsets[scopeStackOffsetIndex] += lhsTypeSize;

	// the lhs should return a pointer, so it'll always be a quad
	Indent(ss);
	ss << "pushq %rax" << endl;

	ExpressionCodeGen(ss, rhs, state);

	// pop the lhs value off the stack into %rdi
	state.scopeStackOffsets[scopeStackOffsetIndex] -= lhsTypeSize;
	state.regAlloc[Registers::RDI] = true;

	Indent(ss);
	ss << "popq %rdi" << endl;

	Indent(ss);
	ss << "mov" << lhsTypeInstSuffix << " " << GetRegNameForType(lhsType, Registers::RAX) << ", (%rdi)" << endl;
}

string GetSymbolPointer(const Symbol& sym, CodeGenState& state)
{
	if (sym.isGlobal) {
		return "$" + sym.identifier;
	} else {
		size_t currStackOffset = state.scopeStackOffsets[state.scopeStackOffsets.size() - 2];
		return intToStr(currStackOffset + sym.stackOffset) + "(%rsp)";
	}
}