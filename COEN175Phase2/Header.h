#include <vector>
#include <set>
#include <string>

#include "Type.h"
#include "Symbol.h"
#include "Tree.h"
#include "Variant.h"

void match(const std::string& tokenType);
void match(const std::string& tokenType, Variant& v);
std::string lookahead(unsigned int ahead = 0);
void Expression();

void TranslationUnit();
void GlobalDeclaration(TreeNode<std::vector<Symbol>>& scope);
void GlobalDeclarator(Symbol& s, eSpecifier spec);
size_t Pointers();
eSpecifier Specifier();
void FunctionDefinition(TreeNode<std::vector<Symbol>>& scope);
void Parameters(std::vector<Symbol*>& params);
void ParameterList(std::vector<Symbol*>& params);
void Parameter(Symbol& s);
void Declarations(TreeNode<std::vector<Symbol>>& scope);
void Declaration(TreeNode<std::vector<Symbol>>& scope);
void DeclaratorList(TreeNode<std::vector<Symbol>>& scope);
void Declarator(Symbol& s, eSpecifier spec);
void Statements(TreeNode<std::vector<Symbol>>& scope);
void Statement(TreeNode<std::vector<Symbol>>& scope);
void ExpressionList();
int	 Number();