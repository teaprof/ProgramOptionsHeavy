#ifndef PARSERS_OPTIONSGROUP_H
#define PARSERS_OPTIONSGROUP_H

#include <OptionsEasy/OptionsEasy.h>

#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <cstdint>

namespace program_options_heavy {

class HeavyOptionBase {
    public:
        HeavyOptionBase() = default;
        virtual void onNewOccurenceFinished(std::vector<std::any>& values) = 0;
        virtual void onParseFinished() = 0;
        virtual void validate() = 0; // can raise exception if values for this option are incorrect

        virtual std::shared_ptr<AbstractOption> opt() = 0;
    protected:
        std::string help_string_;
};

class HeavyPositionalOption : public HeavyOptionBase {
    public:
        HeavyPositionalOption() = default;
};

class HeavyFlag : public HeavyOptionBase {
    public:
        HeavyFlag() = default;
};

class HeavyNamedOptionWithValue : public HeavyOptionBase {
    public:
        HeavyNamedOptionWithValue() = default;
};

class HeavyLiteralCommand : public HeavyOptionBase {
    public:
        HeavyLiteralCommand() = default;
};

class HeavyOptionsGroup : public OptionsEasy { // TODO: why OptionsEay?
   public:
    HeavyOptionsGroup(std::string group_name) {
        setGroupName(tolower(group_name));
        options = std::make_shared<OptionsGroup>();
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
    std::shared_ptr<OptionsGroup> options;

   private:
    std::string group_name_;
};

} /* namespace program_options_heavy */

#endif  // PARSERS_OPTIONSGROUP_H