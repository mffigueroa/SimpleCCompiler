#include <iostream>
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

  /*RegisterAllocation tmp;
  CodeGen(ast, tmp);*/

  delete ast;
  delete parserState;
  return 0;
}