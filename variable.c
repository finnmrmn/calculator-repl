/// @file variable.c
/// @brief The symbol table for storing results with labels.
///
/// This could be implemented as a map, but these values are intended to be
/// manually entered in the REPL. For simplicity, this is left as a list.
/// This can be easily adapted later to use a hash function + lookup.
///
/// @author finnmrmn
/// @date 2026-08-14

#include "variable.h"

#include <ctype.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>

/// @brief A variable is a value with a label in this calculator.
struct Variable {
    char label[VARLEN];
    double value;
};

struct Variable variables[VARMAX];
size_t count;

/// @brief Shifts the cursor through a label.
/// @param anchor The start of the input.
/// @param label The extracted label.
/// @return The updated cursor.
const char *variable_extract_label(const char *anchor,
                                   const char label[VARLEN]) {
    const char *cursor = anchor;
    if (isalpha(*cursor) || *cursor == '_') {
        cursor++;
        while (isalnum(*cursor) || *cursor == '_')
            cursor++;
    }
    size_t varlen = cursor - anchor;
    if (!(0 < varlen && varlen < VARLEN))
        return nullptr;
    if (!strncpy((char *)label, anchor, varlen))
        return nullptr;
    return cursor;
}

/// @brief Get a stored value if it exists.
/// @param label The variable name to search for.
/// @param value The value to update with the stored value.
/// @return True if a value was retrieved.
bool variable_get_value(const char *label, double *value) {
    *value = NAN;
    for (size_t i = 0; i < count; i++)
        if (strcmp(label, variables[i].label) == 0) {
            *value = variables[i].value;
            return true;
        }
    return false;
}

/// @brief Defines or redefines a variable.
/// @param label The name of the variable.
/// @param value The value of the variable.
/// @return True if a variable was defined or redefined.
bool variable_define(const char label[VARLEN], double value) {
    size_t i;
    for (i = 0; i < count; i++)
        if (strcmp(label, variables[i].label) == 0) {
            variables[i].value = value;
            return true;
        }
    if (count == VARMAX)
        return false;
    (void)strcpy(variables[i].label, label);
    variables[i].value = value;
    count++;
    return true;
}

/// @brief Deletes a variable from the list.
/// @param label The name of the variable.
/// @return True if a variable was deleted.
bool variable_delete(const char label[VARLEN]) {
    if (count == 0)
        return false;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(label, variables[i].label) == 0) {
            if (i < count)
                memmove(&variables[i], &variables[count - 1],
                        sizeof(variables[i]));
            count--;
            return true;
        }
    }
    return false;
}