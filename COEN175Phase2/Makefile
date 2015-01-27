all: executable
executable: sourceFile
	g++ -g -o ./scc ./lex.cc ./Util.cpp ./Tokens.cpp ./Variant.cpp ./Type.cpp ./Symbol.cpp ./outputDotFile.cpp ./MainParser.cpp ./ExpressionParser.cpp ./main.cpp
sourceFile:
	flex -o ./lex.cc ./lexer.l
