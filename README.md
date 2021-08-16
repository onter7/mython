# Mython Interpreter
**Mython** (*Mini-python*) - a simplified subset of Python programming language

## Description
### Numbers
Mython uses only integers. You can perform common arithmetic operations with them: addition, subtraction, multiplication, integer division.
### Strings
A string constant in Mython is a sequence of arbitrary characters on one line and delimited by double quotes ```"``` or single ```'```. Escaping of the special characters ```'\n'```, ```'\t'```, ```'\''``` and ```'\"'``` is supported. Examples of strings in Mython:
- ```"hello"```
- ```'world'```
- ```'long string with a double quote " inside'```
- ```"another long string with a single quote ' inside"```
- ```"string with a double quote \" inside"```
- ```'string with a single quote \' inside'```
- ```''```, ```""``` — empty strings.
Strings in Mython are immutable.
### Boolean constants and None
In addition to string and integer values, Mython supports the Boolean values ```True``` and ```False```. There is also a special value ```None```.
### Comments
Mython supports single line comments starting with the ```#``` character. All following text up to the end of the current line is ignored. ```#``` within strings is considered a normal character.
### Identifiers
Mython identifiers are used to denote the names of variables, classes and methods: start with a lowercase or uppercase Latin letter, or with an underscore. This is followed by an arbitrary sequence consisting of numbers, letters and the underscore character.
Examples of valid identifiers: ```x```, ```_42```, ```do_something```, ```int2str```.
Examples of invalid identifiers:
- ```4four``` - starts with a number;
- ```one;two``` - contains a character that is not a number, letter, or underscore.
### Classes
In Mython, you can define your type by creating a class. A class declaration begins with the ```class``` keyword, followed by a name **identifier** and a declaration for the class methods.
### Operations
Mython defines:
- Arithmetic operations for integers, division is performed entirely. Division by zero causes a runtime error.
- String concatenation operation, for example: ```s = 'hello,' + 'world'```.
- Comparison operations for strings and integers ```==```, ```!=```, ```<=```, ```>=```, ```<```, ```>```; comparison of strings is performed lexicographically.
- Logical operations ```and```, ```or```, ```not```.
- Unary minus.
Priority of operations (in descending order of priority):
- Unary minus.
- Multiplication and division.
- Addition and subtraction.
- Comparison operations.
- Logical operations.

The order of evaluation of expressions can be changed by parentheses: 
```
print 2 + 3 * 4   # outputs 14
print (2 + 3) * 4 # outputs 20
```
In Mython, the addition operation, in addition to numbers and strings, is applicable to class objects with a special method ```__add__```.
Comparison operations are applied not only to numbers and strings, but also to objects of classes that have the ```__eq__``` (equal to) and ```__lt__``` (less than) methods. All comparison operations can be implemented using these methods.
### str function
The ```str``` function converts the argument passed to it to a string. If the argument is an object of the class, it calls the special ```__str__``` method on it and returns the result. If there is no ```__str__``` method in the class, the function returns a string representation of the object's memory address.
### print command
The special command ```print``` takes a set of arguments separated by commas, prints them to standard output, and optionally prints a line feed. The ```print``` command inserts a space between the output values. If no arguments are passed to it, it will simply print a newline.
### Conditional operator
Mython has a conditional statement. Its syntax is:
```
if <condition>:
   <action 1>
   <action 2>
   ...
   <action N>
else:
   <action 1>
   <action 2>
   ...
   <action M> 
```
### Inheritance
In Mython, a class can have one parent class. If present, it appears in parentheses after the class name and before the colon character.
### Methods
Methods in Mython have the syntax:
```
def <method name> (<parameter list>):
   <action 1>
   <action 2>
   ...
   <action N> 
```
### Assignment semantics
Mython is a dynamically typed language, so the assignment operation has the semantics of not copying a value into a memory area, but of binding a variable name to a value. As a consequence, variables only refer to values, and do not contain copies of them.
### Other restrictions
The result of calling a method or constructor in Mython is a terminal operation. Its result can be assigned to a variable or used as a parameter of a function or command, but you cannot directly access the fields and methods of the returned object.

## Build
The project supports building using CMake. External dependencies are not used, only the standard library.
