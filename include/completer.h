#ifndef __COMPLETER_H__
#define __COMPLETER_H__

#include <string>
#include <vector>

inline std::vector<std::string> complete(const std::string &program, const std::string &curargument,
                                         const std::string &prevargument)
{
    return {"run", "collect"};
}

#endif