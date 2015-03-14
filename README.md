# SimpleCCompiler
A x86-64 compiler written from scratch in C++ for my compilers course.

This compiler outputs Intel x86-64 assembly code in the AT&T x86 syntax. It compiles the code that is input from standard input
and writes it to standard output and to a file named "output.s".

In order to aid debugging, it also outputs a .dot file representing the Abstract Syntax Tree that can be rendered using graphviz.
