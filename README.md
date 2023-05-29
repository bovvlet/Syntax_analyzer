# Syntax_analyzer
## Written lexical, syntax and semantical analyzer for basic-like language.
Here is it formal grammar:
```
<program> ::= <line> ;\n <program>
<program> ::= <line> $
<line> ::= <number> <command>
<command> ::= <var> := <expression>
<expression> ::= <var>
<expression> ::= <integer>
<expression> ::= <fun> ( <expressions> )
<expressions> ::= <expression> , <expressions>
<expressions> ::= <expression>
<fun> ::= sum
<fun> ::= random_choise
```
In order to check it, run:
```
g++ syntax_analyzer.cpp && ./a.out
```
