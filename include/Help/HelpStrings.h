#ifndef __HELP_HELPSTRINGS_H__
#define __HELP_HELPSTRINGS_H__

#include <Help/TextualDescriptions.h>

#include <Backend/Option.h>

#include <sstream>
#include <map>


class HelpStrings {
    private:
        class HelpBuilder { // TODO remove this
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
        // TODO: add possibility to replace option names with preformatted string
        HelpBuilder operator<<(std::shared_ptr<AbstractOption> opt) {
            return HelpBuilder(help_strings_[opt]);
        }
        void setGroupName(std::shared_ptr<OptionsGroup2> opt, const std::string& str) {
            group_descriptions[opt].name = str;
        }
        std::string getGroupName(std::shared_ptr<OptionsGroup2> opt) const {
            if(group_descriptions.count(opt) == 0) {
                return "grp";
                /*static size_t ccc = 0;
                std::stringstream str;
                str<<"grp"<<ccc++;
                GroupDescription descr{str.str(), {""}, {""}, {}};
                group_descriptions[opt] = descr;*/
            }
            return group_descriptions.at(opt).name;
        }

        void merge(const HelpStrings& other) { // TODO may be unused
            // TODO merge program_description
            
            // some sanity checks:
            for(const auto& it : other.help_strings_)
                assert(help_strings_.count(it.first) == 0);
            for(const auto& it : other.key_strings_)
                assert(key_strings_.count(it.first) == 0);
            for(const auto& it : other.group_descriptions)
                assert(group_descriptions.count(it.first) == 0);

            // do merge
            help_strings_.insert(other.help_strings_.begin(), other.help_strings_.end());
            key_strings_.insert(other.key_strings_.begin(), other.key_strings_.end());
            group_descriptions.insert(other.group_descriptions.begin(), other.group_descriptions.end());
        }
      
        ProgramDescription program_description;
        std::map<std::shared_ptr<OptionsGroup2>, GroupDescription> group_descriptions;
        std::map<std::shared_ptr<AbstractOption>, std::string> help_strings_;
        std::map<std::shared_ptr<AbstractOption>, std::string> key_strings_;
};



#endif