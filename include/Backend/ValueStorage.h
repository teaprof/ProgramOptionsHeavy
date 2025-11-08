#ifndef __BACKEND_VALUE_STORAGE_H__
#define __BACKEND_VALUE_STORAGE_H__

#include <string>


template<class ... T>
class TValueStorage {
    public:

    template<class Arg>
    TValueStorage(Arg& v) : ref{v} {}

    void setValue(const std::string& str) {
        assert(std::holds_alternative<std::reference_wrapper<std::string>>(ref));
        auto r = std::get<std::reference_wrapper<std::string>>(ref);        
        r.get() = str;
    }
    
    std::variant<std::reference_wrapper<T>...> ref;
};

using ValueStorage = TValueStorage<int, double, std::string, char, bool>;


#endif