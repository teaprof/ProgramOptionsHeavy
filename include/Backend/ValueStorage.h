#ifndef __BACKEND_VALUE_STORAGE_H__
#define __BACKEND_VALUE_STORAGE_H__

#include "ValueSemantics.h"
#include <any>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <optional>

class AbstractNamedOptionWithValue;
class PositionalOption;

/*!
 * Value storage do the following things:
 * - stores the origin of the value : default or explicitly set as the cmd line argument
 * - stores string representation of the value if it comes from the cmd line
 * - if supplied writes the typed value to the user supplied reference
 */
class BaseValueStorage {
    public:
        std::vector<std::string> raw_values_;
        std::string raw_value_;
        bool is_defaulted{true};

        void setValue(const std::string& value, BaseValueSemantics& value_semantics, std::optional<std::any> external_value_ptr = std::nullopt) {
            value_semantics.checkValue(value);
            raw_values_.push_back(value);
            raw_value_ = value;
            is_defaulted = false;
            if(external_value_ptr.has_value()) {
                value_semantics.setValue2(value, external_value_ptr.value());
            }
        }
        size_t size() {
            return raw_value_.size();
        }
};


class ValuesStorage {
    public:
    void addCounter(std::shared_ptr<AbstractOption> opt) {
        opts_counter_[opt]++;
    }
    void addValue(std::shared_ptr<AbstractNamedOptionWithValue> opt, const std::string& str) {
        if(values_map_.count(opt) != 0) {
            throw DuplicateOption(opt);
        }
        auto value = std::make_shared<BaseValueStorage>();
        if(external_pointers_.count(opt) > 0) {
            value->setValue(str, opt->baseValueSemantics(), external_pointers_[opt]);
        } else {
            value->setValue(str, opt->baseValueSemantics(), std::nullopt);
        }
        values_map_[opt] = value;
        addCounter(opt);
    }
    void clear() {
        values_map_.clear();
        opts_counter_.clear();
    }
    std::shared_ptr<BaseValueStorage>& operator[](std::shared_ptr<AbstractNamedOptionWithValue> opt) {
        return values_map_[opt];
    }
    bool contains(std::shared_ptr<AbstractNamedOptionWithValue> opt) {
        return values_map_.count(opt) > 0;
    }
    size_t count(std::shared_ptr<AbstractOption> opt) {
        if(opts_counter_.count(opt) > 0)
            return opts_counter_[opt];
        return 0;
    }
    template<class T>
    void setExternalStorage(std::shared_ptr<AbstractOption> opt, T* val_ptr) {
        external_pointers_[opt] = val_ptr;
    }
    private:
    std::map<std::shared_ptr<AbstractNamedOptionWithValue>, std::shared_ptr<BaseValueStorage>> values_map_;
    std::map<std::shared_ptr<AbstractOption>, size_t> opts_counter_;
    std::map<std::shared_ptr<AbstractOption>, std::any> external_pointers_;
        
};



#endif