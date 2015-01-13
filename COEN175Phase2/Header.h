#include <vector>
#include <set>
#include <string>

extern std::vector<std::pair<std::string, std::string>> tokens;

void match(const std::string& tokenType);
std::string lookahead(int ahead = 0);
void expr();

class ParseLevelFunctor;

class ExpressionParser {
public:
	ExpressionParser();
	void operator()();
private:
	ParseLevelFunctor*			m_ninthLvl;
};

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