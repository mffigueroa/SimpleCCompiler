#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <string>
using namespace std;

#include "Tokens.h"
#include "Header.h"
#include "CodeGen.h"

int main()
{
  CreateTokenMap();
  TreeNode<ASTNodeVal>* ast;
  ParserState*			parserState;
  TranslationUnit(&ast, &parserState);

  outputDotFile(ast, "ast.dot");
  renderDotFile("ast.dot");

  stringstream ss;
  InitRegMaps();
  RootCodeGen(ss, ast);

  ofstream fp("output.s", ios::out);
  cout << ss.str();
  fp << ss.str();

  delete ast;
  delete parserState;
  return 0;
}