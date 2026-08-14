#ifndef VARIABLE_H
#define VARIABLE_H

#define VARMAX 16
#define VARLEN 16 /// Null-terminator counts towards this value.

const char *variable_extract_label(const char *anchor,
                                   const char label[VARLEN]);
bool variable_define(const char *label, double value);
bool variable_delete(const char *label);
bool variable_get_value(const char *label, double *value);

#endif