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

        void merge(const Help& other) {
            // TODO brief_program_description
            // TODO detailed_program_description
            
            //some sanity checks:
            for(const auto& it : other.help_strings_)
                assert(help_strings_.count(it.first) == 0);
            for(const auto& it : other.group_names_)
                assert(group_names_.count(it.first) == 0);
            help_strings_.insert(other.help_strings_.begin(), other.help_strings_.end());
            group_names_.insert(other.group_names_.begin(), other.group_names_.end());
        }
      
        std::string brief_program_description;
        std::string detailed_program_description;
        std::map<std::shared_ptr<AbstractOption>, std::string> help_strings_;
        std::map<std::shared_ptr<OptionsGroup2>, std::string> group_names_;
};


class SingleOptionPrinter : public AbstractOptionVisitor {
    public:
        SingleOptionPrinter() {}
        SingleOptionPrinter(const Help& help) : help_(help) {}
        void visit(std::shared_ptr<AbstractOption> opt) {
            // assert(false);
        }
        void visit(std::shared_ptr<AbstractPositionalOption> opt) {
            str<<"some positional option"<<helpString(opt);                    
        }
        void visit(std::shared_ptr<NamedOption> opt) {
            str<<displayName(opt)<<" "<<helpString(opt);
        }
        void visit(std::shared_ptr<LiteralString> opt) {
            str<<opt->str()<<" literal: "<<helpString(opt);
        }
        void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) {
            str<<displayName(opt)<<" "<<helpString(opt);
        }
        void visit(std::shared_ptr<AbstractPositionalOptionWithValue> opt) {
            str<<"some positional option"<<helpString(opt);                    
        }
        void visit(std::shared_ptr<OptionsGroup2> opt) {
            str<<groupName(opt)<<" "<<helpString(opt);
        }
        void visit(std::shared_ptr<OneOf> opt) {
            str<<"one of"<<" "<<helpString(opt);
        }
        std::stringstream str;
    private:
        std::optional<std::reference_wrapper<const Help>> help_;
        std::string displayName(std::shared_ptr<NamedOption> opt) {
            std::string res;
            std::string delimiter;
            if(opt->longName()) {
                res = *opt->longName();
                delimiter = ", ";
            };
            if(opt->shortName()) {
                res += delimiter + *opt->shortName();
            };
            return res;
        }
        std::string helpString(std::shared_ptr<AbstractOption> opt) {
            if(help_) {
                if(help_->get().help_strings_.count(opt) > 0) {
                    return help_->get().help_strings_.at(opt);
                }
            }
            return "";
        }
        std::string groupName(std::shared_ptr<OptionsGroup2> opt) {
            if(help_) {
                if(help_->get().group_names_.count(opt) > 0) {
                    return help_->get().group_names_.at(opt);
                }
            }
            return "[group]";
        }
};


#endif