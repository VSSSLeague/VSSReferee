#ifndef TEXT_H
#define TEXT_H

#include <iostream>

class Text
{
public:
    Text();

    static std::string center(std::string s);
    static std::string bold(std::string s);
    static std::string red(std::string s, bool bold = false);
    static std::string green(std::string s, bool bold = false);
    static std::string yellow(std::string s, bool bold = false);
    static std::string blue(std::string s, bool bold = false);
    static std::string purple(std::string s, bool bold = false);
    static std::string cyan(std::string s, bool bold = false);
};

#endif // TEXT_H
