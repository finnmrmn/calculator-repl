#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdlib.h>

/// @brief A calculation context that may or may not fail.
/// @note Nothing is heap allocated by the parser. Do not free.
/// @note Example usage: struct Calculation calculation = {"1 + sin(pi/2)"};
struct Calculation {
    struct {
        const char *const expression;
    } input;
    struct {
        double value;
    } output;
    struct {
        const char *message;
        size_t column;
        size_t line;
    } error;
};

bool calculate(struct Calculation *calculation);

void read_evaluate_print_loop(void);

#endif // CALCULATOR_H