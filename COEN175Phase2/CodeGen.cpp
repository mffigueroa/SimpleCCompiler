#include <iostream>
#include <list>
#include <algorithm>

using namespace std;

#include "CodeGen.h"

map<string, string> constStrGlobals;

_CodeGenState::_CodeGenState()
{
	for (size_t i = 0; i < Registers::NUM_REG; ++i) {
		regAlloc[i] = false;
	}
}

_StatementGenState::_StatementGenState()
	: numAndOpsInFunc(0), numOrOpsInFunc(0)
{
	expectsAddress.push(false);
	expectsValueIfPointerType.push(false);
}

namespace Registers
{
	vector<size_t>		parameterRegisters;
	map<size_t, string>	regNames;
	set<size_t>			callerSaved;
	vector<size_t>		calleeSaved;
}

void InitRegMaps()
{
	Registers::regNames[Registers::RAX] = "%rax";
	Registers::regNames[Registers::RBX] = "%rbx";
	Registers::regNames[Registers::RCX] = "%rcx";
	Registers::regNames[Registers::RDX] = "%rdx";
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

	Registers::parameterRegisters.push_back(Registers::RDI);
	Registers::parameterRegisters.push_back(Registers::RSI);
	Registers::parameterRegisters.push_back(Registers::RDX);
	Registers::parameterRegisters.push_back(Registers::RCX);
	Registers::parameterRegisters.push_back(Registers::R8);
	Registers::parameterRegisters.push_back(Registers::R9);

	Registers::calleeSaved.push_back(Registers::RBX);
	Registers::calleeSaved.push_back(Registers::R12);
	Registers::calleeSaved.push_back(Registers::R13);
	Registers::calleeSaved.push_back(Registers::R14);
	Registers::calleeSaved.push_back(Registers::R15);

	for (size_t i = 0; i < Registers::NUM_REG; ++i) {
		if (find(Registers::calleeSaved.begin(), Registers::calleeSaved.end(), i) == Registers::calleeSaved.end()) {
			Registers::callerSaved.insert(i);
		}
	}
}

string GetRegNameForType(const Type& t, Registers::Reg reg)
{
	string origName = Registers::regNames[reg];
	size_t typeSize = GetTypeSize(t, false);

	if ('0' <= origName[2] && origName[2] <= '9') {
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
	size_t typeSize = GetTypeSize(t, false);

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
	list<TreeNode<ASTNodeVal>*>& children = node->getChildList();
	list<TreeNode<ASTNodeVal>*>::iterator i = children.begin(), i_end = children.end();

	ss << endl << ".data" << endl << endl;

	// declare our global variables
	for (; i != i_end; ++i) {
		TreeNode<ASTNodeVal>* childNode = *i;
		// generate code for every definition of
		// a function or a global variable
		if (childNode->val.type == ASTNodeValType::SYMBOL) {
			// declare the global variables in the assembly code
			if (!childNode->val.symbol->second.type.isFunction) {
				GlobalVarGen(ss, childNode);
			}
		}
	}

	StringVariantCodeGen(ss, node);

	// start the code part, declaring this to be
	// the .text section
	ss << endl << ".text" << endl << endl;

	i = children.begin(), i_end = children.end();

	for (; i != i_end; ++i) {
		TreeNode<ASTNodeVal>* childNode = *i;
		// generate code for every definition of
		// a function or a global variable
		if (childNode->val.type == ASTNodeValType::SYMBOL) {
			// if this is a function declaration, ignore it. but if its a definition
			// then generate its code.
			if (childNode->val.symbol->second.type.isFunction) {
				if (childNode->getChildList().size() > 0) {
					FuncCodeGen(ss, childNode);
				}
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

	for (size_t i = 0; i < sym.type.arraySize; ++i) {
		if (i > 0) {
			ss << ",";
		}

		ss << "0";
	}

	if (!sym.type.arraySize) {
		ss << "0";
	}

	ss << endl;
}

void FuncCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node)
{
	const Symbol& sym = node->val.symbol->second;

	ss << ".global " << sym.identifier << endl;
	ss << sym.identifier << ":" << endl;
	
	Indent(ss);
	ss << "pushq %rbp" << endl;
	Indent(ss);
	ss << "movq %rsp, %rbp" << endl << endl;

	stringstream blockCode;
	CodeGenState blockState;

	// initialize the state of our parameters
	const vector<Symbol*>& funcParams = node->val.symbol->second.type.funcParams;
	
	size_t currParamStackOffset = 0, paramNum = 0;

	// save the parameters on the stack, but only the ones
	// that aren't already on the stack
	for (vector<Symbol*>::const_iterator i = funcParams.begin(); i != funcParams.end(); ++i, ++paramNum) {
		// since we have to push quads on the stack no matter
		// what the size of the actual variable, every parameter's
		// offset will be a multiple of 8 (the size of a quad)
		currParamStackOffset += 8;
		blockState.funcParameters[*i] = currParamStackOffset;

		Indent(ss);
		ss << "pushq " << Registers::regNames[(Registers::Reg)Registers::parameterRegisters[paramNum]] << endl;

		// only need to push parameters on the stack that aren't already on it.
		// i.e. parameters that were passed in via a register. 
		if (paramNum == Registers::parameterRegisters.size() - 1) {
			break;
		}
	}

	// we allocated all the parameters we just pushed and %rbp (which is of size 8)
	blockState.scopeStackAllocSizes.push_back(8 + currParamStackOffset);

	blockState.enclosingFunc = node->val.symbol;
	blockState.numWhileStatementsInFunc = 0;
	blockState.numIfStatementsInFunc = 0;
	blockState.temporaryStackOffset = 0;

	// the codegenstate passed in here will record
	// all the registers its used so we only have to
	// save them
	StatementGenState stmtState;
	BlockCodeGen(blockCode, node, blockState, stmtState);

	// save the callee saved registers to the stack
	for (size_t i = 0; i < Registers::NUM_REG; ++i) {
		vector<size_t>::const_iterator calleeIt = find(Registers::calleeSaved.begin(), Registers::calleeSaved.end(), i);
		if (blockState.regAlloc[i] && calleeIt != Registers::calleeSaved.end()) {
			Indent(ss);
			ss << "pushq " << Registers::regNames[i] << endl;
		}
	}

	ss << endl << blockCode.str();

	ss << endl << "func_" << sym.identifier << "_end:" << endl;

	// pop the callee saved registers from the stack in reverse order
	for (int i = Registers::NUM_REG - 1; i >= 0; --i) {
		vector<size_t>::const_iterator calleeIt = find(Registers::calleeSaved.begin(), Registers::calleeSaved.end(), i);
		if (blockState.regAlloc[i] && calleeIt != Registers::calleeSaved.end()) {
			Indent(ss);
			ss << "popq " << Registers::regNames[i] << endl;
		}
	}

	paramNum = 0;

	// take the parameters we saved on the stack off of it
	for (vector<Symbol*>::const_iterator i = funcParams.begin(); i != funcParams.end(); ++i, ++paramNum) {
		Indent(ss);
		ss << "popq %rdi" << endl;

		if (paramNum == Registers::parameterRegisters.size() - 1) {
			break;
		}
	}

	ss << endl;
	Indent(ss);
	ss << "movq %rbp, %rsp" << endl;
	Indent(ss);
	ss << "popq %rbp" << endl;
	Indent(ss);
	ss << "ret" << endl;
}

size_t AllocateStackForBlock(TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	const list<TreeNode<ASTNodeVal>*>& children = node->getChildList();
	list<TreeNode<ASTNodeVal>*>::const_iterator declNode = children.begin();

	for (; declNode != children.end(); ++declNode) {
		if ((*declNode)->val.type == ASTNodeValType::VARIANTTYPENODE &&
			(*declNode)->val.variantTypeNode.variant.getStrVal() == "DECLS") {
				break;
		}
	}

	if (declNode == children.end()) {
		return 0;
	}

	const list<TreeNode<ASTNodeVal>*>& decls = (*declNode)->getChildList();
	list<TreeNode<ASTNodeVal>*>::const_iterator currDecl = decls.begin();

	size_t totalDeclsSize = 0;

	vector<SymbolTableRef> currScopeVariables;

	for (; currDecl != decls.end(); ++currDecl) {
		if ((*currDecl)->val.type == ASTNodeValType::SYMBOL) {
			currScopeVariables.push_back((*currDecl)->val.symbol);
			(*currDecl)->val.symbol->second.stackOffset = totalDeclsSize;
			totalDeclsSize += GetTypeSize((*currDecl)->val.symbol->second.type);
		}
	}

	state.scopeVariables.push_back(currScopeVariables);

	return totalDeclsSize;
}

void BlockCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	size_t stackSize = AllocateStackForBlock(node, state, stmtState);

	// in accessing local stack-allocated variables we have to offset rsp by the stack
	// allocation sizes of various scopes
	state.scopeStackAllocSizes.push_back(stackSize);

	if (stackSize) {
		Indent(ss);
		ss << "subq $" << stackSize << ", %rsp" << endl << endl;
	}
	
	const list<TreeNode<ASTNodeVal>*>& children = node->getChildList();
	list<TreeNode<ASTNodeVal>*>::const_iterator it = children.begin();

	for (; it != children.end(); ++it) {
		if ((*it)->val.type == ASTNodeValType::VARIANTTYPENODE && (*it)->val.variantTypeNode.variant.getStrVal() == "STMTS") {
			const list<TreeNode<ASTNodeVal>*>& stmtsChildren = (*it)->getChildList();
			list<TreeNode<ASTNodeVal>*>::const_iterator stmtIt = stmtsChildren.begin(), stmtIt_end = stmtsChildren.end();
			
			for (; stmtIt != stmtIt_end; ++stmtIt) {
				StatementCodeGen(ss, *stmtIt, state, stmtState);
			}
		}
	}

	if (stackSize) {
		ss << endl;
		Indent(ss);
		ss << "addq $" << stackSize << ", %rsp" << endl;
	}

	// get rid of our stack offset as we finish
	// generating code for this block
	state.scopeStackAllocSizes.pop_back();
}

void StatementCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	if (node->val.lineNumber != -1) {
		size_t lineNum = node->val.lineNumber;
		ss << "/* line " << node->val.lineNumber <<  "*/" << endl;
	}

	if (node->val.type == ASTNodeValType::SYMBOL) {
		SymbolAccessCodeGen(ss, node, state, stmtState);
	}

	string nodeName;
	
	if (node->val.type == ASTNodeValType::VARIANTTYPENODE) {
		nodeName = node->val.variantTypeNode.variant.getStrVal();
	} else if (node->val.type == ASTNodeValType::VARIANT) {
		nodeName = node->val.variant.getStrVal();
	}

	if (nodeName == "BLOCK") {
		BlockCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "FUNC_CALL") {
		FuncCallCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "RETURN") {
		ReturnCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "WHILE") {
		WhileCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "IF") {
		IfCodeGen(ss, node, state, stmtState);
	} else if (nodeName == "ASSIGN") {
		AssignCodeGen(ss, node, state, stmtState);
	} else if (find(operatorNames.begin(), operatorNames.end(), nodeName) != operatorNames.end()) {
		ExpressionCodeGen(ss, node, state, stmtState);
	}
}

Type FuncCallCodeGen(std::stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	list<TreeNode<ASTNodeVal>*>& children = node->getChildList();
	list<TreeNode<ASTNodeVal>*>::iterator i = children.begin();

	const Symbol& func = (*i)->val.symbol->second;
	size_t numParams = children.size() - 1;
	size_t numRegisterParams = min(Registers::parameterRegisters.size(), numParams);

	size_t totalStackAllocationInFunc = 0;

	for (size_t i = 0; i < state.scopeStackAllocSizes.size(); ++i) {
		totalStackAllocationInFunc += state.scopeStackAllocSizes[i];
	}

	vector<string>	paramLocation;
	vector<Type>	paramTypes;
	list<TreeNode<ASTNodeVal>*>::iterator paramNode = ++i;

	// won't use any of the callee-saved registers are used in any other
	// part of code generation so we can save them for function calls.

	size_t stackParamsTotalOffset = 0;
	size_t numSavedCalleeSavedRegs = 0;

	for (size_t param = 0; param < numParams; ++param, ++paramNode) {
		Type paramType = ExpressionCodeGen(ss, *paramNode, state, stmtState);
		paramTypes.push_back(paramType);

		if (param < Registers::calleeSaved.size()) {
			string destRegName = GetRegNameForType(paramType, (Registers::Reg)Registers::calleeSaved[param]);
			state.regAlloc[(Registers::Reg)Registers::calleeSaved[param]] = true;

			// push the callee saved register on the stack.
			// otherwise we will overwrite it when we do nested
			// function calls.
			Indent(ss);
			ss << "pushq " << Registers::regNames[Registers::calleeSaved[param]] << endl;
			++numSavedCalleeSavedRegs;
			state.temporaryStackOffset += 8;

			Indent(ss);
			ss << "mov" << GetInstSuffixForType(paramType) << " " << GetRegNameForType(paramType, Registers::RAX)
				<< ", " << destRegName << endl;
			paramLocation.push_back(destRegName);
		} else {
			// push the lhs value on the stack while we evaluate the
			// next expression
			state.temporaryStackOffset += 8;
			stackParamsTotalOffset += 8;

			Indent(ss);
			ss << "pushq %rax" << endl;

			if (param == Registers::calleeSaved.size() + 1) {
				// store the location of the first non-callee saved register (the 6th one)
				// on the stack as we'll be moving this value to a register for a parameter
				string destRegName = "-" + intToStr(totalStackAllocationInFunc + state.temporaryStackOffset);
				destRegName += "(%rbp)";
				paramLocation.push_back(destRegName);
			}
		}
	}

	// move the first six parameters into the correct registers.
	// we don't have to move the rest because they'll be stored in the correct
	// place in the stack.
	for (size_t param = 0; param < numRegisterParams; ++param, --numParams) {
		Indent(ss);
		ss << "mov" << GetInstSuffixForType(paramTypes[param]) << " "
			<< paramLocation[param] << ", "
			<< GetRegNameForType(paramTypes[param], (Registers::Reg)Registers::parameterRegisters[param]) << endl;
	}

	// for functions that take variable numbers of parameters,
	// let them know we don't pass any values in the xmm registers
	Indent(ss);
	ss << "movq $0, %rax" << endl;

	// make the actual function call
	Indent(ss);
	ss << "call " << func.identifier << endl;

	// pop the called saved registers we saved
	for (int i = numSavedCalleeSavedRegs - 1; i >= 0; --i) {
		Indent(ss);
		ss << "popq " << Registers::regNames[Registers::calleeSaved[i]] << endl;
		state.temporaryStackOffset -= 8;
	}

	// if we have more than five parameters then we pushed the sixth one
	// onto the stack and them moved it into a register. so we have to pop it off
	// the stack now. we'll pop it into %rdi since that register's value was already destroyed
	// when we put our first parameter's value in it.
	if (numParams >= Registers::calleeSaved.size()) {
		state.regAlloc[Registers::RDI] = true;
		const Type& paramType = paramTypes[Registers::calleeSaved.size() + 1];

		Indent(ss);
		ss << "popq %rdi" << endl;
	}

	state.temporaryStackOffset -= stackParamsTotalOffset;

	ss << endl;

	Type t;
	t.arraySize = func.type.arraySize;
	t.lvlsOfIndirection = func.type.lvlsOfIndirection;
	t.spec = func.type.spec;
	return t;
}

void ReturnCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	ExpressionCodeGen(ss, *node->getChildList().begin(), state, stmtState);

	Indent(ss);
	ss << "jmp func_" << state.enclosingFunc->second.identifier << "_end" << endl;
}

void WhileCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	list<TreeNode<ASTNodeVal>*>::iterator i = node->getChildList().begin();
	TreeNode<ASTNodeVal>* conditionNode = *i;
	++i;
	TreeNode<ASTNodeVal>* exprNode = *i;
	
	string lblPrefix = ".WHILE_" + state.enclosingFunc->second.identifier + "_" + intToStr(state.numWhileStatementsInFunc) + "_";
	state.numWhileStatementsInFunc++;

	ss << lblPrefix << "START:" << endl;

	Type t = ExpressionCodeGen(ss, conditionNode, state, stmtState);

	Indent(ss);
	string raxName = GetRegNameForType(t, Registers::RAX);
	ss << "test" << GetInstSuffixForType(t) << " " << raxName << ", " << raxName << endl;

	Indent(ss);
	ss << "jz " << lblPrefix << "END" << endl;

	StatementCodeGen(ss, exprNode, state, stmtState);

	Indent(ss);
	ss << "jmp " << lblPrefix << "START" << endl;

	ss << lblPrefix << "END:" << endl;
}

void IfCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	list<TreeNode<ASTNodeVal>*> children = node->getChildList();
	bool hasElseNode = children.size() == 3;

	list<TreeNode<ASTNodeVal>*>::iterator i = children.begin();
	TreeNode<ASTNodeVal>* conditionNode = *i;
	++i;
	TreeNode<ASTNodeVal>* stmtNode = *i;

	Type t = ExpressionCodeGen(ss, conditionNode, state, stmtState);

	string lblPrefix = ".IF_" + state.enclosingFunc->second.identifier + "_" + intToStr(state.numIfStatementsInFunc) + "_";
	state.numIfStatementsInFunc++;

	Indent(ss);
	string raxName = GetRegNameForType(t, Registers::RAX);
	ss << "test" << GetInstSuffixForType(t) << " " << raxName << ", " << raxName << endl;

	Indent(ss);

	if (hasElseNode) {
		ss << "jz " << lblPrefix << "ELSE" << endl;
	} else {
		ss << "jz " << lblPrefix << "END" << endl;
	}

	StatementCodeGen(ss, stmtNode, state, stmtState);

	if (hasElseNode) {
		Indent(ss);
		ss << "jmp " << lblPrefix << "END" << endl;

		ss << lblPrefix << "ELSE:" << endl;

		++i;
		TreeNode<ASTNodeVal>* elseNode = *i;
		StatementCodeGen(ss, *elseNode->getChildList().begin(), state, stmtState);
	}

	ss << lblPrefix << "END:" << endl;
}

void AssignCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	list<TreeNode<ASTNodeVal>*>::iterator i = node->getChildList().begin();
	TreeNode<ASTNodeVal>* lhs = *i;
	++i;
	TreeNode<ASTNodeVal>* rhs = *i;

	// generate code to give the address of the lhs, given its
	// an l-value
	stmtState.expectsAddress.push(true);
	Type lhsType = ExpressionCodeGen(ss, lhs, state, stmtState);
	stmtState.expectsAddress.pop();

	char lhsTypeInstSuffix = GetInstSuffixForType(lhsType);

	// push the lhs value on the stack while we evaluate the
	// next expression
	state.temporaryStackOffset += 8;

	// the lhs should return a pointer, so it'll always be a quad
	Indent(ss);
	ss << "pushq %rax" << endl;

	stmtState.expectsAddress.push(false);
	ExpressionCodeGen(ss, rhs, state, stmtState);
	stmtState.expectsAddress.pop();

	// pop the lhs value off the stack into %rdi
	state.temporaryStackOffset -= 8;
	state.regAlloc[Registers::RDI] = true;

	Indent(ss);
	ss << "popq %rdi" << endl;

	Indent(ss);
	ss << "mov" << lhsTypeInstSuffix << " " << GetRegNameForType(lhsType, Registers::RAX) << ", (%rdi)" << endl;
}

string GetSymbolPointer(const SymbolTableRef symRef, CodeGenState& state)
{
	const Symbol& sym = symRef->second;
	if (sym.isGlobal) {
		return "(" + sym.identifier + ")";
	} else {
		size_t i = 0;

		// find the scope of the variable
		for (; i < state.scopeVariables.size(); ++i) {
			vector<SymbolTableRef>::const_iterator var;
			var = find(state.scopeVariables[i].begin(), state.scopeVariables[i].end(), symRef);

			if (var != state.scopeVariables[i].end()) {
				break;
			}
		}

		// if its a local stack variable
		if (i != state.scopeVariables.size()) {
			// have to add one to the index because
			// the first stack allocation size isn't
			// for local variables but for saved
			// parameters
			++i;

			// add all the allocation sizes of the scopes
			// beneath it to recover the bottom of the stack
			// for the variable's scope
			++i;
			size_t currStackOffset = 0;

			for (; i < state.scopeStackAllocSizes.size(); ++i) {
				currStackOffset += state.scopeStackAllocSizes[i];
			}

			// take RSP and add the temporary stack offset to recover the RSP of the bottom/current scope.
			// then add all the stack sizes of the scopes beneath it to get to the RSP of the stack for the
			// variable's scope. finally add the offset of variable within that scope. this leaves
			// us with with the offset from the current RSP to our variable.
			return intToStr(currStackOffset + state.temporaryStackOffset + sym.stackOffset) + "(%rsp)";
		} else {
			// if its not a local variable, check if its a parameter
			const vector<Symbol*>& enclosingFuncParams = state.enclosingFunc->second.type.funcParams;
			vector<Symbol*>::const_iterator param = enclosingFuncParams.begin();

			for (; param != enclosingFuncParams.end(); ++param) {
				// if this parameter isn't a register-passed parameter saved on the stack,
				// then it was on the stack before the function call and is above %rbp
				map<Symbol*, size_t>::const_iterator it = state.funcParameters.find(*param);
				if (it == state.funcParameters.end()) {
					break;
				}

				if (**param == sym) {
					return "-" + intToStr(it->second) + "(%rbp)";
				}
			}

			size_t rbpOffset = 16;
			for (; param != enclosingFuncParams.end(); ++param) {
				if (**param == sym) {
					return intToStr(rbpOffset) + "(%rbp)";
				}

				rbpOffset += GetTypeSize((*param)->type);
			}
		}
	}
}

Type VariantCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node, CodeGenState& state, StatementGenState& stmtState)
{
	const Variant& v = node->val.variant;
	Variant::VariantType type = v.getType();
	Type t;

	Indent(ss);

	if (type == Variant::INT) {
		t.spec = Type::INT;
		ss << "movl $" << v.getIntVal() << ", %eax" << endl;
	} else if (type == Variant::LONGINT) {
		t.spec = Type::LONGINT;
		ss << "movq $" << v.getIntVal() << ", %rax" << endl;
	} else if (type == Variant::CHAR) {
		t.spec = Type::INT;
		ss << "movl $" << (int)v.getCharVal() << ", %eax" << endl;
	} else if (type == Variant::STRING) {
		t.spec = Type::CHAR;
		t.lvlsOfIndirection = 1;

		map<string, string>::const_iterator it = constStrGlobals.find(v.getStrVal());

		if (it != constStrGlobals.end()) {
			ss << "movq $" << it->second << ", %rax" << endl;
		} else {
			ss << "movq $0, %rax" << endl;
		}
	}

	return t;
}

void StringVariantCodeGen(stringstream& ss, TreeNode<ASTNodeVal>* node)
{
	// found a string variant, declare it as a global variable
	if (node->val.type == ASTNodeValType::VARIANT && node->val.variant.getType() == Variant::STRING) {
		string strVal = node->val.variant.getStrVal();
		map<string, string>::const_iterator it = constStrGlobals.find(strVal);

		// if it hasn't been declared yet,
		// declare it and store the global name
		if (it == constStrGlobals.end()) {
			size_t currNumConstStrs = constStrGlobals.size();
			string newConstStrName = ".ConstStrVal" + intToStr(currNumConstStrs);
			ss << newConstStrName << ":" << endl;
			Indent(ss);
			ss << ".asciz " << strVal << endl;
			constStrGlobals[strVal] = newConstStrName;
		}
	} else {
		list<TreeNode<ASTNodeVal>*>& children = node->getChildList();
		list<TreeNode<ASTNodeVal>*>::iterator i = children.begin(), i_end = children.end();

		// search any children for string variants
		for (; i != i_end; ++i) {
			StringVariantCodeGen(ss, *i);
		}
	}
}