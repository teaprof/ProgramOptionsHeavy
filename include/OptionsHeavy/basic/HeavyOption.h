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

        void setRequired(bool is_required) {}
        void setMaxOccurreneCount(size_t count) {}

        virtual void onNewOccurenceFinished(std::vector<std::any>& values) = 0;
        virtual void onParseFinished() = 0;

        void addUnlocks(HeavyOptionBase& opt) {
            unlocks_.push_back(opt);
        }

        void setHelpString(const std::string& str) {
            help_string_ = str;
        }

        virtual std::shared_ptr<AbstractOption> opt() = 0;
        virtual void generateOpt() = 0;
    protected:
        bool is_encountered{false}; 
        std::string help_string_;
        std::vector<std::reference_wrapper<HeavyOptionBase>> unlocks_;
};


template<class ValueType>
class HeavyNamedOptionWithValue : public HeavyOptionBase {
    public:
        HeavyNamedOptionWithValue() {
            opt_ = std::make_shared<NamedOptionWithValue<ValueType>>();
            opt_->valueSemantics().setExternalValueStorage(value);
        }
        void setDefaultValue(const ValueType& default_value) {
            opt_->valueSemantics().setDefaultValue(default_value);
        }
        void setImplicitValue(const ValueType& implicit_value) {
            opt_->valueSemantics().setImplicitValue(implicit_value);
        }

        void setNValues(AbstractOptionWithValue::NValuesRole role, size_t count = 1) {
            opt_->setNValues(role, count);
        }

        virtual void onDefaultValueApplied(std::vector<std::any>& values) = 0;
        virtual void onImplicitValueApplied(std::vector<std::any>& values) = 0;
        virtual void validateValue() = 0; // can raise exception if values for this option are incorrect

        void generateOpt() override {}
        std::shared_ptr<AbstractOption> opt() { return opt_; }

        ValueType value;
    private:
        std::shared_ptr<NamedOptionWithValue<ValueType>> opt_;
};

template<class ValueType>
class HeavyPositionalOptionWithValue : public HeavyOptionBase {
    public:
        void setImplicitValue(std::any implicit_value) {}

        void setNValues(AbstractOptionWithValue::NValuesRole role, size_t count = 1);

        virtual void onImplicitValueApplied(std::vector<std::any>& values) = 0;
        virtual void validateValue() = 0; // can raise exception if values for this option are incorrect
};

class HeavyFlag : public HeavyOptionBase {
    public:
        HeavyFlag() = default;
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