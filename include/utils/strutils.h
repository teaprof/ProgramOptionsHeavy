#ifndef __UTILS_STRUTILS_H__
#define __UTILS_STRUTILS_H__

#include <Grammar/Parser.h> // for ArgLexer::short_options_pattern and ArgLexer::long_option_pattern

#include <cassert>
#include <map>
#include <optional>
#include <regex>
#include <string>
#include <vector>

std::pair<bool, std::string> isLongName(std::string name);
std::pair<bool, std::string> isShortName(std::string name);

inline std::pair<std::optional<std::string>, std::optional<std::string>> splitToLongAndShortNames(const std::string& names) {
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

inline std::pair<bool, std::string> isLongName(std::string name) {
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

inline std::pair<bool, std::string> isShortName(std::string name) {
    if(name[0] != '-') {
        name = std::string("-") + name;
    }
    std::regex regex(ArgLexer::short_options_pattern);
    if(std::regex_match(name, regex))  {
        return {true, name.substr(1, std::string::npos)};
    }
    return {false, ""};
}

inline std::string trim(const std::string& src) {
    if(src.empty()) {
        return src;
    }
    size_t start = 0;
    while(start < src.size() && std::isspace(src[start]))
        start++;
    size_t end = src.length() - 1;
    while(end > start && std::isspace(src[end]))
        end--;
    return src.substr(start, end - start + 1);
}

inline std::string tolower(const std::string& src) {
    std::string lower;
    for (auto ch : src)
    {
        lower += std::tolower(ch);
    }
    return lower;
}

#endif