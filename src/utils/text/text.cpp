#include "text.h"

Text::Text() {

}

std::string Text::center(std::string s) {
    int terminalWidth = 80;
    int spaces = (terminalWidth - s.length()) / 2;

    std::string out;

    if(spaces > 0) {
        out += std::string(spaces, ' ');
    }

    out += s;

    return out;
}

std::string Text::bold(std::string s) {
    return "\033[1m" + s + "\033[0m";
}

std::string Text::red(std::string s, bool bold) {
    if(bold) {
        return "\033[1;31m" + s + "\033[0m";
    }
    else {
        return "\033[0;31m" + s + "\033[0m";
    }
}

std::string Text::green(std::string s, bool bold) {
    if(bold) {
        return "\033[1;32m" + s + "\033[0m";
    }
    else {
        return "\033[0;32m" + s + "\033[0m";
    }
}

std::string Text::yellow(std::string s, bool bold) {
    if(bold) {
        return "\033[1;33m" + s + "\033[0m";
    }
    else {
        return "\033[0;33m" + s + "\033[0m";
    }
}

std::string Text::blue(std::string s, bool bold) {
    if(bold) {
        return "\033[1;34m" + s + "\033[0m";
    }
    else {
        return "\033[0;34m" + s + "\033[0m";
    }
}

std::string Text::purple(std::string s, bool bold) {
    if(bold) {
        return "\033[1;35m" + s + "\033[0m";
    }
    else {
        return "\033[0;35m" + s + "\033[0m";
    }
}

std::string Text::cyan(std::string s, bool bold) {
    if(bold) {
        return "\033[1;36m" + s + "\033[0m";
    }
    else {
        return "\033[0;36m" + s + "\033[0m";
    }
}
