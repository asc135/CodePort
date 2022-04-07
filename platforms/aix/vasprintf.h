#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

// This code originated from a StackOverflow answer at the following URL:
// https://stackoverflow.com/questions/4899221/substitute-or-workaround-for-asprintf-on-aix
// 04/07/2022
//

extern int asprintf(char **ret, const char *format, ...);
extern int vasprintf(char **ret, const char *format, va_list args);