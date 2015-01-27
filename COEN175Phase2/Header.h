#include <list>
#include <map>
#include <vector>
#include <set>
#include <string>

#include "Type.h"
#include "Symbol.h"
#include "Tree.h"
#include "Variant.h"

typedef std::map<std::string, Symbol>	Scope;
typedef std::list<Scope>				ScopeStack;

void match(const std::string& tokenType, Variant* v = 0, unsigned int* lineNumber = 0);
std::string lookahead(unsigned int ahead = 0);

bool LookupSymbol(const ScopeStack& stack, const std::string& symbolName, Symbol* r_symbol = 0);
void outputError(unsigned int lineNumber, const std::string& err);
void outputDotFile(TreeNode<Variant>* v, const std::string& filename);
std::string GetSpecifierName(eSpecifier spec);

TreeNode<Variant>* TranslationUnit();

size_t Pointers();
eSpecifier Specifier();
int	 Number();

TreeNode<Variant>*				GlobalDeclaration(ScopeStack& variableStack, ScopeStack& functionStack);
TreeNode<Variant>*				GlobalDeclarator(ScopeStack& variableStack, ScopeStack& functionStack, eSpecifier spec);
TreeNode<Variant>*				FunctionDefinition(ScopeStack& variableStack, ScopeStack& functionStack);
TreeNode<Variant>*				Parameters(ScopeStack& variableStack, ScopeStack& functionStack, std::vector<Symbol*>& parameters);
TreeNode<Variant>*				ParameterList(ScopeStack& variableStack, ScopeStack& functionStack);
TreeNode<Variant>*				Parameter(ScopeStack& variableStack, ScopeStack& functionStack, Symbol& s);
TreeNode<Variant>*				Declarations(ScopeStack& variableStack, ScopeStack& functionStack);
std::list<TreeNode<Variant>*>	Declaration(ScopeStack& variableStack, ScopeStack& functionStack);
TreeNode<Variant>*				DeclaratorList(ScopeStack& variableStack, ScopeStack& functionStack);
TreeNode<Variant>*				Declarator(ScopeStack& variableStack, ScopeStack& functionStack, eSpecifier spec);
TreeNode<Variant>*				Statements(ScopeStack& variableStack, ScopeStack& functionStack);
TreeNode<Variant>*				Statement(ScopeStack& variableStack, ScopeStack& functionStack);
std::list<TreeNode<Variant>*>	ExpressionList(ScopeStack& variableStack, ScopeStack& functionStack);
TreeNode<Variant>*				Expression(ScopeStack& variableStack, ScopeStack& functionStack);