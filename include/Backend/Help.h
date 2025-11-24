#ifndef __BACKEND_HELP_H__
#define __BACKEND_HELP_H__

#include <Backend/Option.h>

#include <sstream>
#include <map>

class Help {
    private:
        class HelpBuilder {
            public:
                HelpBuilder(std::string& dest) : dest_{dest} {}
                ~HelpBuilder() {
                    dest_ = stream.str();
                }
                template<class T>
                HelpBuilder& operator<<(T&& val) {
                    stream<<val;
                    return *this;
                }
            private:
                std::stringstream stream;
                std::string& dest_;
        };
    public:
        HelpBuilder operator<<(std::shared_ptr<AbstractOption> opt) {
            return HelpBuilder(help_strings_[opt]);
        }
        void setGroupName(std::shared_ptr<OptionsGroup2> opt, const std::string& str) {
            group_names_[opt] = str;
        }
      
        std::string brief_program_description;
        std::string detailed_program_description;
        std::map<std::shared_ptr<AbstractOption>, std::string> help_strings_;
        std::map<std::shared_ptr<OptionsGroup2>, std::string> group_names_;
};



#endif