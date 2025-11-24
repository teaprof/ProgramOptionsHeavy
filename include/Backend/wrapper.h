#ifndef __BACKED_WRAPPER_H__
#define __BACKED_WRAPPER_H__

/*#include "ValueSemantics.h"
#include "Option.h"
#include "Parser.h"*/

#include "Matcher.h"

#include <map>
#include <optional>
#include <string>
#include <regex>

std::pair<std::optional<std::string>, std::optional<std::string>> splitToLongAndShortNames(const std::string& names);
std::pair<bool, std::string> isLongName(std::string name);
std::pair<bool, std::string> isShortName(std::string name);


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


std::pair<std::optional<std::string>, std::optional<std::string>> splitToLongAndShortNames(const std::string& names) {
    constexpr char separator=',';
    assert(names.find(' ') == std::string::npos);
    size_t sep_pos = names.find(separator);
    std::vector<std::string> v;
    if(sep_pos != std::string::npos) {
        std::string first = names.substr(0, sep_pos);
        std::string second = names.substr(sep_pos+1, std::string::npos);
        if(first.size() > 0) {
            v.push_back(first);
        }
        if(second.size() > 0) {
            v.push_back(second);
        }        
    } else {
        v.push_back(names);
    }
    std::optional<std::string> long_name, short_name;
    for(auto it : v) {
        auto [is_long, stripped_long_name] = isLongName(it);
        if(is_long) {
            long_name = stripped_long_name;
            continue;
        } 
        auto [is_short, stripped_short_name] = isShortName(it);
        if(is_short) {
            short_name = stripped_short_name;
            continue;
        } 
    }
    return {long_name, short_name};
}

std::pair<bool, std::string> isLongName(std::string name) {
    if(name[0] != '-') {
        if(name.length() == 1) {
            return {false, ""};
        }
        name = std::string("--") + name;
    }
    std::regex regex(ArgLexer::long_option_pattern);
    if(std::regex_match(name, regex)) {
        return {true, name.substr(2, std::string::npos)};
    }
    return {false, ""};
}

std::pair<bool, std::string> isShortName(std::string name) {
    if(name[0] != '-') {
        name = std::string("-") + name;
    }
    std::regex regex(ArgLexer::short_options_pattern);
    if(std::regex_match(name, regex))  {
        return {true, name.substr(1, std::string::npos)};
    }
    return {false, ""};
}

#endif