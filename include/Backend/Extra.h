#ifndef __BACKEND_EXTRA_H__
#define __BACKEND_EXTRA_H__

#include "Option.h"
#include <string>

class NamedCommand : public NamedOption {
public:          
    NamedCommand(const std::string& command_name) : NamedOption(command_name) {}
};

#endif