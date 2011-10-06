#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "utils.h"

int get_integer_digit(int len) {
  int i;

  for (i = 0; len != 0; i++) {
    len /= 10;
  }

  return i;
}

char* new_concat_strings(int len, ...) {
  va_list strings;
  int i;
  char* string;
  int all_len;
  char* result;

  va_start(strings, len);
  all_len = 0;
  for (i = 0; i < len; i++) {
    string = va_arg(strings, char*);
    all_len += strlen(string);
  }
  va_end(strings);

  result = calloc(all_len + 1, 1);
  va_start(strings, len);
  for (i = 0; i < len; i++) {
    string = va_arg(strings, char*);
    strcat(result, string);
  }
  va_end(strings);

  result[all_len] = '\0';
  return result;
}
