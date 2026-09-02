#ifndef BACKEND_FACADE_H
#define BACKEND_FACADE_H

#include <Backend/Option.h>
#include <Backend/ValueSemantics.h>
#include <Backend/ValueStorage.h>
#include <Backend/Parser.h>
#include <Checker/Checker.h>
#include <Completer/Completer2.h>
#include <Help/HelpStringsStorage.h>
#include <utils/strutils.h>

class OptionsEasy {
   public:
    OptionsEasy() {
        root_ = std::make_shared<AbstractOption>();
    }
    template <class T>
    std::shared_ptr<NamedOptionWithValue<T>> makeOption(const std::string& names,
                                                        const std::string& help_message) {
        auto res = makeOptionWithName<NamedOptionWithValue<T>>(names);        
        help_ << res << help_message;
        root_->addUnlock(res);
        return res;
    }

    template <class T>
    std::shared_ptr<NamedOptionWithValue<T>> makeOption(const std::string& names,
                                                        std::reference_wrapper<std::optional<T>> storage,
                                                        const std::string& help_message) {
        auto res = makeOptionWithName<NamedOptionWithValue<T>>(names);
        //res->valueSemantics().setExternalStorage(storage); // TODO: implement this
        help_ << res << help_message;
        root_->addUnlock(res);
        return res;
    }

    template <class T>
    std::shared_ptr<NamedOptionWithValue<T>> makeOption(const std::string& names,
                                                        std::reference_wrapper<T> storage,
                                                        const std::string& help_message) {
        auto res = makeOptionWithName<NamedOptionWithValue<T>>(names);
        res->valueSemantics().setExternalStorage(storage);
        help_ << res << help_message;
        root_->addUnlock(res);
        return res;
    }

    template <class T>
    std::shared_ptr<PositionalOptionWithValue<T>> makeOption(std::reference_wrapper<T> storage, std::string& help_message) {
        auto res = std::make_shared<PositionalOptionWithValue<T>>();
        res->valueSemantics().setExternalStorage(storage);
        help_ << res << help_message;
        root_->addUnlock(res);
        return res;
    }

    template <class T>
    std::shared_ptr<PositionalOptionWithValue<T>> makePositionalOption(std::reference_wrapper<T> external_storage, size_t max_occurrence,
                                                                       const std::string& help_message) {
        auto res = std::make_shared<PositionalOptionWithValue<T>>();
        res->valueSemantics().setExternalStorage(external_storage);
        res->valueSemantics().setMaxOccurrence(max_occurrence);        
        help_ << res << help_message;
        root_->addUnlock(res);
        return res;
    }

    void merge(const OptionsEasy& other) { 
        for(auto& it : other.root_->unlocks()) {
            root_->addUnlock(it);
        }
        help_.merge(other.help_); 
    }
    const HelpStringsStorage& help() { return help_; }

    void parse(std::vector<std::string> args) {
        Parser parser{root_};
    }

    std::vector<std::string> getCompletionVariants(std::vector<std::string> args) {
        Completer completer{root_};
        return completer.getCompletionVariants(args);
    }
    void check() {
        Checker checker;
        checker(root_);
    }

    std::shared_ptr<AbstractOption> opt() { return root_; };
    

   protected:
    HelpStringsStorage help_;
    ValueStorage storage_;
    std::shared_ptr<AbstractOption> root_;

    template <class OptionClass>
    std::shared_ptr<OptionClass> makeOptionWithName(const std::string& names) {
        auto [long_name, short_name] = splitToLongAndShortNames(names);
        std::shared_ptr<OptionClass> res;
        if (!long_name.has_value()) {
            throw std::logic_error("long option name is unspecified");
        }
        if (short_name.has_value()) {
            res = std::make_shared<OptionClass>(long_name.value(), short_name.value());
        } else {
            res = std::make_shared<OptionClass>(long_name.value());
        }
        return res;
    }
};

#endif