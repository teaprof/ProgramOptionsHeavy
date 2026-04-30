#ifndef PARSERS_OPTIONSGROUP_H
#define PARSERS_OPTIONSGROUP_H

#include <OptionsEasy/OptionsEasy.h>

#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <cstdint>

namespace program_options_heavy {

class Option {
    public:
        Option(std::shared_ptr<AbstractOption> opt) : opt_{opt} {}

        virtual void onNewOccurenceFinished() {
            // nothing to do
            // in inherited classes this function can throw any exception        
        }
        virtual void onParsingFinished() {
            // nothing to do
        }

    private:
        std::shared_ptr<AbstractOption> opt_;
};

template<class T>
class TypedOption : public Option { // TODO: rename to NamedOptionWithValue but this name is already assigned to another class
    public:
        TypedOption(std::string undecorated_long_name, std::reference_wrapper<T> storage) :
            Option(std::make_shared<NamedOptionWithValue<T>>(undecorated_long_name)), storage_{storage} {}

        virtual void validate() {
            // nothing to do
            // in inherited classes this function can throw any exception        
        }
    private:
        std::reference_wrapper<T> storage_;
};

class OptionsGroup : public OptionsEasy {
   public:
    OptionsGroup(std::string group_name) {
        setGroupName(tolower(group_name));
        options = std::make_shared<OptionsGroup2>();
    }

    template <class... Args>
    auto addPartial(Args... args) {
        auto opt = makeOption(args...);
        options->addUnlock(opt);

        return opt;
    }
    template <class T>
    auto addPositional(std::string name, int count, std::reference_wrapper<T> external_storage) {
        auto opt = makePositionalOption(name, count, external_storage);
        options->addUnlock(opt);

        return opt;
    }
    virtual void validate()  // TODO: remove it?
    {
        // nothing to do
        // redefine this function in the derived class
    }
    virtual void update(const boost::program_options::variables_map& vm)  // TODO: remove it?
    {
        // nothing to do
        // redefine this function in the derived class
    }
    void setGroupName(std::string str) {
        help_.setGroupName(options, str);
        group_name_ = std::move(str);
    }
    const std::string& groupName() const { return group_name_; }
    void setGroupDescription(std::string str) { help_ << options << str; }
    std::string description() {
        if (help_.help_strings.contains(options)) {
            return help_.help_strings[options];
        }
        return "";
    }
    std::shared_ptr<OptionsGroup2> options;

   private:
    std::string group_name_;
};

} /* namespace program_options_heavy */

#endif  // PARSERS_OPTIONSGROUP_H