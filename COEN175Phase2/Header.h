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

void match(const std::string& tokenType);
void match(const std::string& tokenType, Variant& v);
std::string lookahead(unsigned int ahead = 0);

bool LookupSymbol(const ScopeStack& stack, const std::string& symbolName, Symbol* r_symbol = 0);
void outputError(const std::string& err);
void outputDotFile(TreeNode<Variant>* v, const std::string& filename);
std::string GetSpecifierName(eSpecifier spec);

TreeNode<Variant>* TranslationUnit();

size_t Pointers();
eSpecifier Specifier();

TreeNode<Variant>*				GlobalDeclaration(ScopeStack& stack);
TreeNode<Variant>*				GlobalDeclarator(ScopeStack& stack, eSpecifier spec);
TreeNode<Variant>*				FunctionDefinition(ScopeStack& stack);
TreeNode<Variant>*				Parameters(ScopeStack& stack, std::vector<Symbol*>& parameters);
TreeNode<Variant>*				ParameterList(ScopeStack& stack);
TreeNode<Variant>*				Parameter(ScopeStack& stack, Symbol& s);
TreeNode<Variant>*				Declarations(ScopeStack& stack);
std::list<TreeNode<Variant>*>	Declaration(ScopeStack& stack);
TreeNode<Variant>*				DeclaratorList(ScopeStack& stack);
TreeNode<Variant>*				Declarator(ScopeStack& stack, eSpecifier spec);
TreeNode<Variant>*				Statements(ScopeStack& stack);
TreeNode<Variant>*				Statement(ScopeStack& stack);
std::list<TreeNode<Variant>*>	ExpressionList(ScopeStack& stack);
TreeNode<Variant>*				Expression(ScopeStack& stack);
int	 Number();