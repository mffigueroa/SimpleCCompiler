all: executable
executable: sourceFile
	g++ -g -o ./scc ./lex.cc ./Util.cpp ./Tokens.cpp ./MainParser.cpp ./ExpressionParser.cpp ./main.cpp
sourceFile:
	flex -o ./lex.cc ./lexer.l
