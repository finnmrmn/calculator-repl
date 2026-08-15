# calculator-repl
A lightweight calculator REPL written entirely in C using recursive-descent parsing.

## Project Demo

![Project Demo](demo.gif)

Example of a test you can try for yourself:
- sqrt(3^2+4^2)
- var n = 8
- var r = 3
- fac(n)/(fac(r) * fac(n - r))
- var theta = pi/3
- sin(theta)^2 + cos(theta)^2
- fib(8)
- var x = e
- var x = ln(x)
- var x = ln(x)
- del x
- x
- del x
- exit

## Features

- Interactivity via REPL
- Error detection
- Error description
- Early recursion escape on error
- Functions (e.g., trigonometric)
- Caret index notation
- Variables
- Doxygen support

Please read CFG.txt for a comprehensive definition of the calculator's language features.

## Requirements

- C23
- GCC 14+ (or another suitable compiler)

## Acknowledgements
Generative AI was used to check my work, help refactor, explain libraries, and pinpoint needle-in-a-haystack bugs.
