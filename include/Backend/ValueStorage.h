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

        virtual void setValue(BaseValueSemantics& value_semantics, std::shared_ptr<SemanticParseResult> parse_result, const std::string& raw_value, std::optional<std::any> external_value_ptr = std::nullopt) {            
            raw_values_.push_back(raw_value);
            raw_value_ = raw_value;
            is_defaulted = false;
            if(external_value_ptr.has_value()) {
                value_semantics.storeTo(parse_result, external_value_ptr.value());
            }
        }
        size_t size() {
            return raw_value_.size();
        }
};

template<class T>
class TypedValueStorage : public BaseValueStorage {
    public:
        T value;

        void setValue(BaseValueSemantics& value_semantics, std::shared_ptr<SemanticParseResult> parse_result, const std::string& raw_value, std::optional<std::any> external_value_ptr = std::nullopt) override {
            BaseValueStorage::setValue(value_semantics, parse_result, raw_value, external_value_ptr);
            value_semantics.storeTo(parse_result, &value);
        }
};


class ValuesStorage {
    public:
    void addValue(std::shared_ptr<AbstractNamedOptionWithValue> opt, const std::string& raw_value, std::shared_ptr<SemanticParseResult> parse_result) {
        auto value = std::make_shared<BaseValueStorage>();
        if(external_pointers_.count(opt) > 0) {
            value->setValue(opt->baseValueSemantics(), parse_result, raw_value, external_pointers_[opt]);
        } else {
            value->setValue(opt->baseValueSemantics(), parse_result, raw_value, std::nullopt);
        }
        values_map_[opt] = value;
    }
    void clear() {
        values_map_.clear();
    }
    std::shared_ptr<BaseValueStorage>& operator[](std::shared_ptr<AbstractNamedOptionWithValue> opt) {
        return values_map_[opt];
    }
    bool contains(std::shared_ptr<AbstractNamedOptionWithValue> opt) {
        return values_map_.count(opt) > 0;
    }
    template<class T>
    void setExternalStorage(std::shared_ptr<AbstractOption> opt, T* val_ptr) {
        external_pointers_[opt] = val_ptr;
    }
    // TODO: add setExternalStorage as std::vector
    private:
    std::map<std::shared_ptr<AbstractNamedOptionWithValue>, std::shared_ptr<BaseValueStorage>> values_map_;
    std::map<std::shared_ptr<AbstractOption>, std::any> external_pointers_;
        
};



#endif