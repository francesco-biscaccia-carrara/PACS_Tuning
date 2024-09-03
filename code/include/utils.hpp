#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <string.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdarg>
#include <cstdlib>

#ifndef MH_VERBOSE
#define MH_VERBOSE 1
#endif

#define EPSILON 1e-7
#define SEED 2120934

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

enum LOG_LEVEL {ERROR, WARN, INFO };

void print_state(LOG_LEVEL type_mess, const char* s, ...);

#endif