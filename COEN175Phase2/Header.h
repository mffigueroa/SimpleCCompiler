#include <vector>
#include <set>
#include <string>

void match(const std::string& tokenType);
std::string lookahead(unsigned int ahead = 0);
void Expression();

void TranslationUnit();
void GlobalDeclaration();
void GlobalDeclarator();
void Pointers();
void Specifier();
void FunctionDefinition();
void Parameters();
void ParameterList();
void Parameter();
void Declarations();
void Declaration();
void DeclaratorList();
void Declarator();
void Statements();
void Statement();
void ExpressionList();
void Number();