#include <list>
#include <iostream>
#include <fstream>
#include <map>
#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include "Header.h"
#include "Variant.h"
#include "Tree.h"

using namespace std;

void printVariant(const Variant& v, string& id, string& label);
void printSymbol(const Symbol& s, string& id, string& label);
void escapeQuotes(string& str);
string outputNodeDotNotation(TreeNode<ASTNodeVal>* node, map<string, int>& idUsage, ofstream& fp, int nodeLevel = 1);

void outputDotFile(TreeNode<ASTNodeVal>* root, const std::string& filename)
{
	ofstream fp(filename, ios::out);

	map<string, int> idUsage;

	fp << "digraph AST {" << endl;
	outputNodeDotNotation(root, idUsage, fp);
	fp << "}";
}

void renderDotFile(const string& filename)
{
#ifdef _WIN32
	size_t bufSize = GetCurrentDirectory(0, NULL);
	string currDirectory;
	currDirectory.resize(bufSize - 1);
	GetCurrentDirectory(bufSize, (char*)currDirectory.c_str());

	string dotRendererPath = "\"C:\\Program Files (x86)\\Graphviz2.38\\bin\\dot.exe\"";
	string cmdLineOptions = " -Tpng \"-o";
	string imgFilename = currDirectory + "\\" + filename.substr(0, filename.find_last_of('.')) + ".png";

	cmdLineOptions += imgFilename + "\" \"" + currDirectory + "\\" + filename + "\"";
	string cmd = dotRendererPath + cmdLineOptions;

	char* cmdBuf = (char*)malloc(cmd.length() + 1);
	strcpy_s(cmdBuf, cmd.length() +1, cmd.c_str());

	STARTUPINFO si, si2;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(&si2, sizeof(STARTUPINFO));
	si.cb = si2.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;

	int ret = CreateProcess(NULL, cmdBuf, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi);

	if (!ret) {
		int errorCode = GetLastError();
		__asm int 3;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	cmd = "rundll32 \"C:\\Program Files\\Windows Photo Viewer\\PhotoViewer.dll\", ImageView_Fullscreen " + imgFilename;

	char* cmdBuf2 = (char*)malloc(cmd.length() + 1);
	strcpy_s(cmdBuf2, cmd.length() + 1, cmd.c_str());

	ret = CreateProcess(NULL, cmdBuf2, NULL, NULL, FALSE, NULL, NULL, NULL, &si2, &pi);

	if (!ret) {
		int errorCode = GetLastError();
		__asm int 3;
	}

	free(cmdBuf);
	free(cmdBuf2);
#endif
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

	newNodeName += "_";
	newNodeName += intToStr(idUsage[nodeName]);;

	return newNodeName;
}

string outputNodeDotNotation(TreeNode<ASTNodeVal>* node, map<string, int>& idUsage, ofstream& fp, int nodeLevel)
{
	for (int i = 0; i < nodeLevel; ++i) {
		fp << "\t";
	}

	string parentNode, parentNodeLbl;
	
	if (node->val.isSymbol) {
		printSymbol(node->val.symbol->second, parentNode, parentNodeLbl);
		parentNode = GetNodeName(parentNode, idUsage);
	} else {
		printVariant(node->val.variant, parentNode, parentNodeLbl);
		parentNode = GetNodeName(parentNode, idUsage);
	}

	escapeQuotes(parentNodeLbl);
	parentNodeLbl = "\"" + parentNodeLbl + "\"";

	escapeQuotes(parentNode);
	parentNode = "\"" + parentNode + "\"";


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

void getSpecifierStr(Type::eSpecifier spec, string& id, string& label)
{
	if (spec == Type::INT) {
		label = "int";
		id = "INT";
	} else if (spec == Type::LONGINT) {
		label = "long";
		id = "LONGINT";
	} else if (spec == Type::CHAR) {
		label = "char";
		id = "CHAR";
	} else if (spec == Type::STRING) {
		label = "const char*";
		id = "STRING";
	} else if (spec == Type::UNDEFINED) {
		label = "undefined";
		id = "UNDEFINED";
	}
}

void printVariant(const Variant& v, string& id, string& label)
{
	if (v.getType() == Variant::STRING || v.getType() == Variant::IDENTIFIER) {
		id = label = v.getStrVal();
	} else if (v.getType() == Variant::CHAR) {
		label = "char('" + string(1, v.getCharVal()) + "')";
		id = "CHAR_" + label;
	} else if (v.getType() == Variant::INT || v.getType() == Variant::LONGINT) {
		id = label = intToStr(v.getIntVal());
		if (v.getType() == Variant::INT) {
			id = "INT_" + id;
			label = "int(" + label + ")";
		} else {
			id = "LONGINT_" + id;
			label = "long(" + label + ")";
		}
	} else if (v.getType() == Variant::SPECIFIER) {
		getSpecifierStr(v.getSpecifierVal(), id, label);
	} else {
		label = "Undefined Variant";
		id = "UNDEFINED";
	}
}

void printSymbol(const Symbol& s, string& id, string& label)
{
	getSpecifierStr(s.type.spec, id, label);

	for (size_t i = 0; i < s.type.lvlsOfIndirection; ++i) {
		label += "*";
		id += "_PTR";
	}

	if (s.type.isFunction && s.type.arraySize > 0) {
		label += "[" + intToStr(s.type.arraySize) + "]";
		id += "_ARR" + intToStr(s.type.arraySize);
	}

	label += " " + s.identifier;
	id += "_" + s.identifier;

	if (!s.type.isFunction && s.type.arraySize > 0) {
		label += "[" + intToStr(s.type.arraySize) + "]";
		id += "_ARR" + intToStr(s.type.arraySize);
	}

	if (s.type.isFunction) {
		label += "(";
		
		vector<Symbol*>::const_iterator i_begin = s.type.funcParams.begin(), i_end = s.type.funcParams.end(), i = i_begin;
		for (; i != i_end; ++i) {
			if (i != i_begin) {
				label += ", ";
			}

			string paramLabel, paramID;
			printSymbol(**i, paramID, paramLabel);

			label += paramLabel;
			id += "_PARAM_" + paramLabel;
		}

		label += ")";
	}
}

void escapeQuotes(string& str)
{
	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == '"') {
			str.insert(i, 1, '\\');
			++i;
		}
	}
}