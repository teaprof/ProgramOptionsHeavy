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

class ValuesStorage {
    public:
    void addValue(std::shared_ptr<AbstractOptionWithValue> opt, const std::string& raw_value) {        
        if(values_map_.count(opt) == 0) {
            //v = std::make_shared<BaseValueStorage>();
            //v = opt->baseValueSemantics().createStorage();
            auto storage = std::make_shared<TypedValueStorage<int>>(); // TODO replace int
            values_map_.insert(std::make_pair(opt, storage));
        };
        values_map_[opt]->setRawValue(raw_value);
        /*if(external_pointers_.count(opt) > 0) {
            v->setValue(parse_result, raw_value, external_pointers_[opt]);
        } else {
            v->setValue(parse_result, raw_value, std::nullopt);
        }*/
    }
    void setDefault(std::shared_ptr<AbstractOptionWithValue> opt, bool flag) {
        assert(values_map_.count(opt) > 0);
        values_map_[opt]->is_defaulted = flag;
    }
    void clear() {
        values_map_.clear();
    }
    std::shared_ptr<BaseValueStorage>& operator[](std::shared_ptr<AbstractOptionWithValue> opt) {
        return values_map_[opt];
    }
    bool contains(std::shared_ptr<AbstractOptionWithValue> opt) {
        return values_map_.count(opt) > 0;
    }
    template<class T>
    void setExternalStorage(std::shared_ptr<AbstractOptionWithValue> opt, T* val_ptr) {
        external_pointers_[opt] = val_ptr;
    }
    // TODO: add setExternalStorage for std::vector<T> support
    private:
    std::map<std::shared_ptr<AbstractOptionWithValue>, std::shared_ptr<BaseValueStorage>> values_map_;
    std::map<std::shared_ptr<AbstractOptionWithValue>, std::any> external_pointers_;
        
};



#endif