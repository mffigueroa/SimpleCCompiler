#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
using namespace std;

#include "Tokens.h"
#include "Header.h"

int main()
{
  CreateTokenMap();
  TreeNode<ASTNodeVal>* ast = TranslationUnit();

  //outputDotFile(ast, "ast.dot");
  return 0;
}