#ifndef BACKEND_VALUESTORAGE_H
#define BACKEND_VALUESTORAGE_H

#include "ValueSemantics.h"
#include <any>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class AbstractNamedOptionWithValue;
class AbstractPositionalOption;

class ValueStorage
{
  public:
    void startNewOccurrence()
    {
        values_.push_back({});
        raw_values_.push_back({});
    }
    void addValueToCurrentOccurence(const std::string &raw_value, const std::any &value)
    {
        assert(!values_.empty());
        values_.back().push_back(value);
        raw_values_.back().push_back(raw_value);
    }
    void add(const std::string &raw_value, const std::any &value)
    {
        startNewOccurrence();
        addValueToCurrentOccurence(raw_value, value);
    }
    template <class T> const T &valueAs() const
    {
        assert(!values_.empty());
        assert(!values_.back().empty());
        return std::any_cast<const T &>(values_.back().back());
    }
    template <class T> const T &valueAs(size_t idx) const
    {
        assert(!values_.empty());
        assert(idx < values_.back().size());
        return std::any_cast<const T &>(values_.back()[idx]);
    }
    template <class T> const T &valueAs(size_t occurrence, size_t idx) const
    {
        assert(idx < values_.size());
        return std::any_cast<const T &>(values_[occurrence][idx]);
    }
    const std::string &rawValues(size_t idx) const
    {
        assert(!raw_values_.empty());
        assert(idx < raw_values_[0].size());
        return raw_values_.back()[idx];
    }
    const std::string &rawValues(size_t occurrence, size_t idx) const
    {
        assert(occurrence < raw_values_.size());
        assert(idx < raw_values_[occurrence].size());
        return raw_values_[occurrence][idx];
    }
    const std::string &rawValue() const
    {
        assert(!raw_values_.empty());
        return raw_values_.back().back();
    }
    size_t lastOccurrenceSize()
    {
        assert(!values_.empty());
        return values_.back().size(); // equals to raw_values_.back().size()
    }
    size_t occurrenceCount()
    {
        return values_.size();
    }
    size_t occurrenceSize(size_t occurrence)
    {
        return values_[occurrence].size();
    }
    // TODO: add setExternalStorage for std::vector<T> support
    // TODO: add hidden names for positional options
  private:
    // values_[i][j] is the j-th value in the i-th occurrence of the option
    // e.g. for "-x 1 2 3 -a -b -x 4" we have values_.size() = 2, values_[0][1] = 2, values[1][0] = 4
    std::vector<std::vector<std::any>> values_;
    // indicies have the same meaning as for values_
    std::vector<std::vector<std::string>> raw_values_;
};

/// key is an option name, value is object of type Value storage
class KeyValueStorage
{
  public:
    void addValue(std::shared_ptr<AbstractOptionWithValue> opt, const std::string &raw_value, const std::any value)
    {
        value_storage_[opt].add(raw_value, value);
        if (external_pointers_.contains(opt))
        {
            opt->baseValueSemantics().store(value, external_pointers_[opt]);
        };
    }
    void addValueToCurrentOccurence(std::shared_ptr<AbstractOptionWithValue> opt, const std::string &raw_value,
                                    const std::any value)
    {
        value_storage_[opt].addValueToCurrentOccurence(raw_value, value);
        if (external_pointers_.contains(opt))
        {
            opt->baseValueSemantics().store(value, external_pointers_[opt]);
        };
    }
    /*void setDefault(std::shared_ptr<AbstractOptionWithValue> opt, bool flag) {
        assert(values_map_.count(opt) > 0);
        values_map_[opt]->is_defaulted = flag;
    }*/
    void clear()
    {
        value_storage_.clear();
    }
    bool contains(std::shared_ptr<AbstractOptionWithValue> opt) const
    {
        return value_storage_.contains(opt);
    }
    template <class T> void setExternalStorage(std::shared_ptr<AbstractOptionWithValue> opt, T *val_ptr)
    {
        external_pointers_[opt] = val_ptr;
    }
    ValueStorage &operator[](std::shared_ptr<AbstractOptionWithValue> opt)
    {
        assert(value_storage_.count(opt) > 0);
        return value_storage_[opt];
    }
    // TODO: add setExternalStorage for std::vector<T> support
  private:
    std::map<std::shared_ptr<AbstractOptionWithValue>, ValueStorage> value_storage_;
    std::map<std::shared_ptr<AbstractOptionWithValue>, std::any> external_pointers_;
};

#endif