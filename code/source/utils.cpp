#include "../include/utils.hpp"

void print_state(LOG_LEVEL type, const char* msg, ...){
    std::string type_msg;
    std::string type_clr;

    switch (type) {
        case ERROR:
            type_clr = ANSI_COLOR_RED; // ANSI_COLOR_RED
            type_msg = "ERROR";
            break;

        case WARN:
            type_clr = ANSI_COLOR_YELLOW ; // ANSI_COLOR_YELLOW
            type_msg = "WARNING";
            break;

        case INFO:
            type_clr = ANSI_COLOR_BLUE; // ANSI_COLOR_BLUE
            type_msg = "INFO";
            break;

        default:
            type_clr = ANSI_COLOR_RESET; // ANSI_COLOR_RESET
            type_msg = "";
            break;
    }

    std::cout << type_clr << "\033[1m\033[4m" << type_msg << ANSI_COLOR_RESET << type_clr << ": ";

    va_list ap;
    va_start(ap, msg);
    vprintf(msg,ap);
    va_end(ap);

    std::cout << ANSI_COLOR_RESET << std::endl;

    if (type == ERROR) std::exit(1);
}