/// @file calculator.c
/// @brief The recursive-descent parser and evaluator for the calculator.
///
/// This is a recursive-descent parser that directly returns a calculated
/// result. By building nodes (terminals) and branches (non-terminals) on a
/// 'root' note instead of a double precision number, an abstract syntax tree
/// tree can be formed and used for deeper semantic analysis; however, in this
/// case, it is not necessary for a calculator with a simple grammar.
///
/// @author finnmrmn
/// @date 2026-08-14

#include "calculator.h"
#include "variable.h"

#include <ctype.h>
#include <float.h> /// For NAN in C23.
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

/// @brief Where the parser is currently reading.
const char *cursor;

/// @brief The output value after parsing and evaluating.
double result;

/// @brief Enables the parser to escape deep recursion on syntax error.
jmp_buf syntax_error;
const char *error_message;

/// @brief Copy of the start of the input. This is used to locate the error.
const char *input_expression;
size_t error_column;
size_t error_line;

/// @brief Skips over any whitespace characters until first non-whitespace.
void whitespace(void) {
    while (isspace(*cursor))
        cursor++;
}

/// @brief Wrapper function that enforces parsing and context invariants.
///
/// If the parser succeeds, then the context is updated and the cursor is
/// progressed. It backtracks as far as necessary if the parser fails to match.
///
/// @param parser The parser being applied to the cursor. Returns true if
/// successful.
/// @return True if parser succeeded.
bool parse(bool (*parser)(void)) {
    whitespace();
    const char *cursor_copy = cursor;
    double result_copy = result;
    if (parser())
        return true;
    cursor = cursor_copy;
    result = result_copy;
    return false;
}

/// @brief Parse a single character.
/// @param character The character being matched at the cursor.
/// @return True if successful.
bool chr(char character) {
    whitespace();
    if (*cursor != character)
        return false;
    cursor++;
    return true;
}

/// @brief Parse a string.
/// @param string The string being matched at the cursor.
/// @return True if successful.
bool str(const char *string) {
    whitespace();
    const char *cursor_copy = cursor;
    while (*string) {
        if (*string != *cursor) {
            cursor = cursor_copy;
            return false;
        }
        cursor++;
        string++;
    }
    return true;
}

/// @brief Allows early escape from deep recursion.
/// @param message The error message that provides more information.
void error(char message[]) {
    result = NAN;
    error_message = message;
    const char *left = input_expression;
    const char *right = cursor;
    error_line = 1;
    error_column = 1;
    while (left < right) {
        if (*left == '\n') {
            error_line++;
            error_column = 1;
        } else {
            error_column++;
        }
        left++;
    }
    longjmp(syntax_error, 1);
}

/// @brief Number parser.
///
/// This is a terminal in the context-free grammar.
/// It also supports a couple of transcendental numbers: 'pi' and 'e'.
/// If nothing matches, then it checks the symbol table.
///
/// @return True if number matched.
bool number(void) {
    const char *anchor = cursor;
    if (str("pi"))
        result = 3.1415926535897931;
    else if (chr('e'))
        result = 2.7182818284590452;
    else if (isdigit(*cursor)) { // match: ^\d+
        cursor++;
        while (isdigit(*cursor))
            cursor++;
        if (*cursor == '.') { // match: ^\d+\.\d*
            cursor++;
            while (isdigit(*cursor))
                cursor++;
        }
        result = strtod(anchor, nullptr);
    } else if (*cursor == '.') { // match: ^\.\d+
        cursor++;
        while (isdigit(*cursor))
            cursor++;
        result = strtod(anchor, nullptr);
    } else
        return false;
    return true;
}

/// @brief The arithmetical operations that can be performed by the evaluator.
enum Operation { NONE, ADD, SUB, MUL, DIV, MOD, POW };

/// @brief Evaluates a binary calculation.
double evaluate(double left, enum Operation op, double right) {
    switch (op) {
    case ADD:
        return left + right;
    case SUB:
        return left - right;
    case MUL:
        return left * right;
    case DIV:
        return left / right;
    case MOD:
        return fmod(left, right);
    case POW:
        return pow(left, right);
    default:
        return NAN;
    }
}

bool expression(void);

double factorial(double n) { return tgamma(n + 1); }

double fibonacci(double n) {
    double sqrt5 = sqrt(5);
    return (pow((1 + sqrt5) / 2, n) - pow((1 - sqrt5) / 2, n)) / sqrt5;
}

/// @brief Function name to function mapping for evaluation.
///
/// @warning Ensure the strings can fit!
/// @warning Substrings can match first, so put shorter strings last!
const struct {
    const char name[8];
    double (*const addr)(double);
} func[] = {{"arcsinh", asinh}, {"arccosh", acosh}, {"arctanh", atanh},
            {"arcsin", asin},   {"arccos", acos},   {"arctan", atan},
            {"round", round},   {"trunc", trunc},   {"gamma", tgamma},
            {"log10", log10},   {"floor", floor},   {"sqrt", sqrt},
            {"cbrt", cbrt},     {"sinh", sinh},     {"cosh", cosh},
            {"tanh", tanh},     {"ceil", ceil},     {"log2", log2},
            {"sin", sin},       {"cos", cos},       {"tan", tan},
            {"abs", fabs},      {"fac", factorial}, {"fib", fibonacci},
            {"ln", log}};

/// @brief Function parser.
///
/// Ensures the function captures an expression enclosed in parentheses.
///
/// @return True if successful.
bool function_expression(void) {
    for (size_t i = 0; i < sizeof(func) / sizeof(*func); i++)
        if (str(func[i].name)) {
            if (!chr('('))
                error("Expected opening parenthesis for function.");
            if (!parse(expression))
                error("Expected expression enclosed in parentheses");
            result = func[i].addr(result);
            if (!chr(')'))
                error("Missing a closing parenthesis for function.");
            return true;
        }
    return false;
}

bool variable(void) {
    const char label[VARLEN] = {0};
    if (!(cursor = variable_extract_label(cursor, label)))
        error("Variable is definition invalid (expression syntax error).");
    if (!variable_get_value(label, &result))
        error("Variable not found.");
    return true;
}

/// @brief Recursion branch either stops or continues from here.
/// @return True if successful.
bool primary_expression(void) {
    if (parse(number)) { // End recursion branch.
    } else if (chr('(')) {
        if (!parse(expression)) // Continue recursion branch.
            error("Expected an expression inside the parentheses.");
        if (!chr(')'))
            error("Expected a closing parenthesis.");
    } else if (parse(function_expression)) { // Continue recursion branch.
    } else if (parse(variable)) {            // End recursion branch.
    } else
        return false;
    return true;
}

bool unary_expression(void);

/// @brief Powers or caret index notation parser.
///
/// This binary operator has higher precedence than all other binary operators
/// implemented right now.
///
/// @return True if successful.
bool power_expression(void) {
    if (!parse(primary_expression))
        return false;
    double base = result;
    if (chr('^')) {
        if (!parse(unary_expression))
            error("Expected an expression after power operator.");
        result = evaluate(base, POW, result);
    }
    return true;
}

/// @brief Unary minus and unary plus parser.
/// @return True if successful.
bool unary_expression(void) {
    if (chr('-')) {
        if (!parse(unary_expression))
            error("Expected an expression after '-'.");
        result = -result;
        return true;
    }
    if (chr('+')) {
        if (!parse(unary_expression))
            error("Expected an expression after '+'.");
        return true;
    }
    return parse(power_expression);
}

/// @brief Multiplication-division-modulo level parser.
/// @return True if successful.
bool multiplicative_expression(void) {
    if (!parse(unary_expression))
        return false;
    for (;;) {
        enum Operation op = NONE;
        if (chr('*'))
            op = MUL;
        else if (chr('/'))
            op = DIV;
        else if (str("mod"))
            op = MOD;
        else
            return true;
        double left = result;
        if (!parse(unary_expression))
            error("Expected operand after operator.");
        result = evaluate(left, op, result);
    }
}

/// @brief Addition-subtraction level parser.
/// @return True if successful.
bool additive_expression(void) {
    if (!parse(multiplicative_expression))
        return false;
    for (;;) {
        enum Operation op = NONE;
        if (chr('+'))
            op = ADD;
        else if (chr('-'))
            op = SUB;
        else
            return true;
        double left = result;
        if (!parse(multiplicative_expression))
            error("Expected operand after operator.");
        result = evaluate(left, op, result);
    }
}

/// @brief The beginning of the calculator parser.
/// @return True if successful.
bool expression(void) { return parse(additive_expression); }

void save_error(struct Calculation *calculation) {
    calculation->output.value = NAN;
    calculation->error.message = error_message;
    calculation->error.column = error_column;
    calculation->error.line = error_line;
}

void save_result(struct Calculation *calculation) {
    calculation->output.value = result;
    calculation->error.message = nullptr;
    calculation->error.column = 0;
    calculation->error.line = 0;
}

/// @brief Calculates a mathematical expression.
/// @param calculation The context of the calculation.
/// @return True if the function succeeds.
bool calculate(struct Calculation *calculation) {
    int has_syntax_error = setjmp(syntax_error);
    if (has_syntax_error) {
        save_error(calculation);
        return false;
    }
    input_expression = calculation->input.expression;
    cursor = calculation->input.expression;
    (void)parse(expression);
    save_result(calculation);
    return true;
}

/// @brief Read a line of input from the user.
/// @return True if input is obtained successfully.
bool repl_get_input(void) {
    static char input[BUFSIZ] = {0};
    printf("-> ");
    if (fgets(input, sizeof(input), stdin) == nullptr) {
        printf("No input found.\n");
        return false;
    }
    cursor = input;
    return true;
}

/// @brief Exit command for the REPL.
/// @return True if exit command is performed.
bool repl_exit(void) {
    if (!str("exit"))
        return false;
    printf("Good bye.\n");
    return true;
}

/// @brief Display the REPL help to the user.
/// @return True if help command is performed.
bool repl_help(void) {
    if (!str("help"))
        return false;
    printf("Close program: exit\n"
           "Define or redefine a variable: var <label> = "
           "<expression> (always use 'var')\n"
           "Evaluate an expression: <expression> (e.g., sqrt(2^3)).\n"
           "Delete variable: del <label>\n");
    return true;
}

/// @brief REPL calculate command.
/// @param result The result of the calculation.
/// @return True if sound result was obtained.
bool repl_calculate(double *result) {
    struct Calculation calculation = {cursor};
    if (!calculate(&calculation)) {
        printf("%s\nError near line %zu, column %zu.\n",
               calculation.error.message, calculation.error.line,
               calculation.error.column);
        return false;
    }
    printf("%lf\n", calculation.output.value);
    if (result)
        *result = calculation.output.value;
    return true;
}

/// @brief REPL delete command.
/// @return True if the command is performed.
bool repl_delete_variable(void) {
    if (!str("del ")) // The suffixed space matters.
        return false;
    char label[VARLEN] = {0};
    if (!(cursor = variable_extract_label(cursor, label))) {
        printf("Variable name invalid (too long / missing).\n");
        return true;
    }
    if (!variable_delete(label)) {
        printf("Nothing to delete.\n");
        return true;
    }
    return true;
}

/// @brief REPL create (or replace) variable command.
/// @return True if the command is performed.
bool repl_create_variable(void) {
    if (!str("var ")) // The suffixed space matters.
        return false;
    char label[VARLEN] = {0};
    whitespace();
    if (!(cursor = variable_extract_label(cursor, label))) {
        printf("Variable name invalid (too long / missing).\n");
        return true;
    }
    if (!chr('=')) {
        printf("Missing '=' after variable label.\n");
        return true;
    }
    double result;
    if (repl_calculate(&result))
        if (!variable_define(label, result)) {
            printf("Too many variables.\n");
            return true;
        }
    // whitespace();
    return true;
}

/// @brief A single iteration through the REPL.
/// @return True if another iteration should happen. False if terminating REPL.
bool repl_iteration(void) {
    if (!repl_get_input())
        return true;
    if (repl_exit())
        return false;
    if (repl_help() || repl_create_variable() || repl_delete_variable())
        return true;
    repl_calculate(nullptr); // Implicitly do calculation on no command.
    return true;
}

/// @brief Calculator REPL.
void read_evaluate_print_loop(void) {
    printf("Calculator REPL. Type 'help' to view commands.\n");
    while (repl_iteration())
        ;
}