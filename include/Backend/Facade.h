#ifndef __BACKEND_FACADE_H__
#define __BACKEND_FACADE_H__

#include <Backend/ValueSemantics.h>
#include <Backend/Option.h>
#include <Backend/Help.h>
#include <utils/strutils.h>

class OptionsFacade {
    public:
        template<class T>
        void addNamed(const std::string& names, std::reference_wrapper<T> val, const std::string& help) {
            //opt<<help;
        }
    private:
        Help help_;
};

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
    (void)help_message;
    res->valueSemantics().setExternalStorage(storage);
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
    (void)help_message;
    return res;
}


template<class T>
std::shared_ptr<PositionalOptionWithValue<T>> makeOption(std::reference_wrapper<T> storage, std::string& help_message) {
    std::shared_ptr<PositionalOptionWithValue<T>> res;
    (void)help_message;
    return res;
}

#endif