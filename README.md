# Calc

This simple calculator works on floating point numbers written with an optional exponent part (**e**). It supports basic operators: **`+`**, **`-`**, **`*`**, **`/`**, **`^`** (power), **`%`** (modulo), **unary `-`**. Sub-expression can be computed using **parenthesis**. 

Mathematical constants **`pi`** and **`e`** are defined as well as a number of basic functions (**`abs`**, **`pow`**, **`sqrt`**, **`exp`**, **`log`** with optional base, **`sin`**, **`cos`**, **`tan`**).

The result of computing an expression can be assigned to a variable using **`>`**. A variable name starts with a letter, and is composed of letters, digit, and undescore **`_`**. Any name can be used as long as it is not the name of a predefined constant or function. 

### Grammar

``` 
digit = ('0'|'1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9')
letter = ('a'..'z'|'A'..'Z'|'_')

number ::= digit+ ('.' digit+)? ('e' ('+'|'-')? digit+)?
name ::= letter+ (digit|letter)*
identifier ::= name ('(' (expr (',' expr)*)? ')')?
primary ::= number|identifier|('(' expr ')')
factor ::= primary ('^' primary)?
term ::= factor (('*'|'/'|'%') factor)*
expr ::= ('+'|'-')? term (('+'|'-') term)*
statement ::= expr ('>'  name)
```
