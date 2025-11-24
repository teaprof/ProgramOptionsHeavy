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
class AbstractPositionalOption;

class ValueStorage {
    public:
    void add(const std::string& raw_value, const std::any& value) {
        values_.push_back(value);
        raw_values_.push_back(raw_value);
    }
    template<class T>
    const T& valueAs() const {
        assert(values_.size() > 0);
        return std::any_cast<const T&>(values_.back());
    }
    template<class T>
    const T& valueAs(size_t idx) const {
        assert(idx < values_.size());
        return std::any_cast<const T&>(values_[idx]);
    }
    size_t valuesCount() const {
        return values_.size();
    }
    const std::string& rawValues(size_t idx) const {
        assert(idx < raw_values_.size());
        return raw_values_[idx];
    }
    const std::string& rawValue() const {
        assert(raw_values_.size() > 0);
        return raw_values_.back();
    }
    size_t size() {
        return values_.size(); // equals to raw_values_.size()
    }
    // TODO: add setExternalStorage for std::vector<T> support
    // TODO: add hidden names for positional options
    private:
    std::vector<std::any> values_;
    std::vector<std::string> raw_values_;
};

class ValuesStorage {
    public:
    void addValue(std::shared_ptr<AbstractOptionWithValue> opt, const std::string& raw_value, const std::any value) {
        value_storage_[opt].add(raw_value, value);
        if(external_pointers_.count(opt) > 0) {
            opt->baseValueSemantics().store(value, external_pointers_[opt]);
        };
    }
    /*void setDefault(std::shared_ptr<AbstractOptionWithValue> opt, bool flag) {
        assert(values_map_.count(opt) > 0);
        values_map_[opt]->is_defaulted = flag;
    }*/
    void clear() {
        value_storage_.clear();
    }
    bool contains(std::shared_ptr<AbstractOptionWithValue> opt) const {
        return value_storage_.count(opt) > 0;
    }
    template<class T>
    void setExternalStorage(std::shared_ptr<AbstractOptionWithValue> opt, T* val_ptr) {
        external_pointers_[opt] = val_ptr;
    }
    ValueStorage& operator[](std::shared_ptr<AbstractOptionWithValue> opt) {
        assert(value_storage_.count(opt) > 0);
        return value_storage_[opt];
    }
    // TODO: add setExternalStorage for std::vector<T> support
    private:
    std::map<std::shared_ptr<AbstractOptionWithValue>, ValueStorage> value_storage_;
    std::map<std::shared_ptr<AbstractOptionWithValue>, std::any> external_pointers_;      
};



#endif