#include <list>
#include <iostream>
#include <fstream>
#include <map>
#include <string>

#include "Header.h"
#include "Variant.h"
#include "Tree.h"

using namespace std;

string printVariant(const Variant& v);
string outputNodeDotNotation(TreeNode<ASTNodeVal>* node, map<string, int>& idUsage, ofstream& fp, int nodeLevel = 1);

void outputDotFile(TreeNode<ASTNodeVal>* root, const std::string& filename)
{
	ofstream fp(filename, ios::out);

	map<string, int> idUsage;

	fp << "digraph AST {" << endl;
	outputNodeDotNotation(root, idUsage, fp);
	fp << "}";
}

string GetNodeName(const string& nodeName, map<string, int>& idUsage)
{
	if (idUsage.find(nodeName) == idUsage.end()) {
		idUsage[nodeName] = 0;
	}
	else {
		idUsage[nodeName]++;
	}

	string newNodeName = nodeName;

	char buf[256];
	_itoa_s(idUsage[nodeName], buf, 256, 10);

	newNodeName += "_";
	newNodeName += buf;

	return newNodeName;
}

string outputNodeDotNotation(TreeNode<ASTNodeVal>* node, map<string, int>& idUsage, ofstream& fp, int nodeLevel)
{
	for (int i = 0; i < nodeLevel; ++i) {
		fp << "\t";
	}

	string parentNodeLbl = printVariant(node->val.variant);
	string parentNode = GetNodeName(parentNodeLbl, idUsage);

	fp << parentNode << " [label = " << parentNodeLbl << "];" << endl;

	const list<TreeNode<ASTNodeVal>*>& children = node->getChildList();

	vector<string> childNodes;

	for (list<TreeNode<ASTNodeVal>*>::const_iterator i = children.begin(), i_end = children.end();
		i != i_end; ++i) {
		childNodes.push_back(outputNodeDotNotation(*i, idUsage, fp, nodeLevel + 1));
	}

	vector<string>::const_iterator childNodeIt = childNodes.begin();
	for (list<TreeNode<ASTNodeVal>*>::const_iterator i = children.begin(), i_end = children.end();
		i != i_end; ++i, ++childNodeIt) {
			for (int j = 0; j < nodeLevel; ++j) {
				fp << "\t";
			}
			fp << parentNode << " -> " << *childNodeIt << ";" << endl;
	}

	return parentNode;
}

string printVariant(const Variant& v)
{
	if (v.getType() == Variant::STRING || v.getType() == Variant::IDENTIFIER) {
		return v.getStrVal();
	} else if (v.getType() == Variant::CHAR) {
		return string(v.getCharVal(), 1);
	} else if (v.getType() == Variant::INT || v.getType() == Variant::LONGINT) {
		char buf[256];
		_itoa_s(v.getIntVal(), buf, 256, 10);
		return "int_" + string(buf);
	} else {
		return "Undefined Variant";
	}
}

void printSymbol(const Symbol& s, string& id, string& label)
{
	;
}