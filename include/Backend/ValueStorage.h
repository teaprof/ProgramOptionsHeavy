#ifndef __BACKEND_VALUE_STORAGE_H__
#define __BACKEND_VALUE_STORAGE_H__

#include <any>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <optional>

class NamedOptionWithValue;
class PositionalOption;

class BaseValue {

};

struct Value : public BaseValue {
    std::string raw_value_;
    std::any value;
    bool is_defaulted;
    
    std::optional<std::any> reference; // Usage: *std::any_cast<T*> = value
};


class ValueStorage {
    public:
    void setValue(std::shared_ptr<NamedOptionWithValue> opt, std::string& str) {
    }
    private:
    std::map<std::shared_ptr<NamedOptionWithValue>, std::shared_ptr<BaseValue>> valuesmap;
        
};



#endif