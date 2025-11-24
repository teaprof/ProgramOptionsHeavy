#ifndef __BACKEND_FACADE_H__
#define __BACKEND_FACADE_H__

#include <Backend/ValueSemantics.h>
#include <Backend/Option.h>
#include <Help/HelpStrings.h>
#include <utils/strutils.h>

class OptionsFacade {
    public:
        template<class T>
        std::shared_ptr<NamedOptionWithValue<T>> makeOption(const std::string& names, std::reference_wrapper<T> storage, const std::string& help_message) {
            auto [long_name, short_name] = splitToLongAndShortNames(names);
            std::shared_ptr<NamedOptionWithValue<T>> res;
            if(!long_name.has_value()) {
                throw std::logic_error("long option name is unspecified");
            }
            if(short_name.has_value()) {
                res = std::make_shared<NamedOptionWithValue<T>>(long_name.value(), short_name.value());
            } else {
                res = std::make_shared<NamedOptionWithValue<T>>(long_name.value());
            }
            res->valueSemantics().setExternalStorage(storage);
            help_<<res<<help_message;
            return res;
        }

        template<class T>
        std::shared_ptr<NamedOptionWithValue<T>> makeOption(const std::string& names, std::reference_wrapper<std::optional<T>> storage, const std::string& help_message) {
            auto [long_name, short_name] = splitToLongAndShortNames(names);
            std::shared_ptr<NamedOptionWithValue<T>> res;
            if(!long_name.has_value()) {
                throw std::logic_error("long option name is unspecified");
            }
            if(short_name.has_value()) {
                res = std::make_shared<NamedOptionWithValue<T>>(long_name.value(), short_name.value());
            } else {
                res = std::make_shared<NamedOptionWithValue<T>>(long_name.value());
            }
            help_<<res<<help_message;
            return res;
        }

        template<class T>
        std::shared_ptr<PositionalOptionWithValue<T>> makeOption(std::reference_wrapper<T> storage, std::string& help_message) {
            auto res = std::make_shared<PositionalOptionWithValue<T>>();
            res->valueSemantics().setExternalStorage(storage);
            help_<<res<<help_message;
            return res;
        }


        template<class T>
        std::shared_ptr<PositionalOptionWithValue<T>> makePositionalOption(const std::string& hidden_name, size_t max_occurrence, std::reference_wrapper<T> external_storage, const std::string& help_message) {
            auto res = std::make_shared<PositionalOptionWithValue<T>>();
            res->valueSemantics().setExternalStorage(external_storage);
            res->valueSemantics().setMaxOccurrence(max_occurrence);
            (void)hidden_name; // TODO
            help_<<res<<help_message;
            return res;
        }

        void merge(const OptionsFacade& other) {
            help_.merge(other.help_);
        }
        const HelpStrings& help() {
            return help_;
        }
    protected:
        HelpStrings help_;
};


//TODO:
// 1. HeavyParser - all featured parser
// 2. SimpleParser - limited parser (alternatives can't have children)
// 3. ParserWithSubcommands (alternatives are allowed only at command line start)

#endif