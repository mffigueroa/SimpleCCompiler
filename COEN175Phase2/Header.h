#include <vector>
#include <set>
#include <string>

extern std::vector<std::pair<std::string, std::string>> tokens;

void match(const std::string& tokenType);
std::string lookahead();
void expr();

class ParseLevelFunctor;

class ExpressionParser {
public:
	ExpressionParser();
	void operator()();
private:
	ParseLevelFunctor*			m_ninthLvl;
};