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
  TreeNode<Variant>* ast = TranslationUnit();

  outputDotFile(ast, "ast.dot");
  return 0;
}