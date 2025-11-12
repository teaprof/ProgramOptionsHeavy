#ifndef __BACKEND_SEMANTIC_PARSE_RESULT_H__
#define __BACKEND_SEMANTIC_PARSE_RESULT_H__

#include <any>
#include <string>
#include <memory>
#include <optional>
#include <vector>

class AbstractOption;

struct SemanticParseResult {
    virtual ~SemanticParseResult() {};
    std::vector<std::shared_ptr<AbstractOption>> unlocks; // what options does the specified value unlock
};

template<class T>
struct TypedSemanticParseResult : public SemanticParseResult {
    T value;    

    TypedSemanticParseResult(const T& v) : value{v} {}
    void store(T& external_storage) {
        external_storage = value;
    }        
        
};

/*!
 * Value storage do the following things:
 * - stores the origin of the value : default or explicitly set as the cmd line argument
 * - stores string representation of the value if it comes from the cmd line
 * - if supplied writes the typed value to the user supplied reference
 */
class BaseValueStorage { // TODO rename this file
    public:
        bool is_defaulted{false};

        virtual void setRawValue(const std::string& raw_value) {
            raw_values_.push_back(raw_value);
            raw_value_ = raw_value;
            is_defaulted = false;
        }
        virtual void setValue(std::shared_ptr<SemanticParseResult> parse_result, const std::string& raw_value, std::optional<std::any> external_value_ptr = std::nullopt) = 0;

        size_t size() const {
            return raw_values_.size();
        }
        const std::string rawValue() const {
            return raw_value_;
        }
        const std::string& rawValues(size_t idx) const {
            return raw_values_[idx];
        }
    private:
        std::vector<std::string> raw_values_;
        std::string raw_value_;
};

template<class T>
class TypedValueStorage : public BaseValueStorage {
    public:
        void setValue(std::shared_ptr<SemanticParseResult> parse_result, const std::string& raw_value, std::optional<std::any> external_value_ptr = std::nullopt) override {
            BaseValueStorage::setRawValue(raw_value);
            auto typed_parse_result = std::dynamic_pointer_cast<TypedSemanticParseResult<T>>(parse_result);
            assert(typed_parse_result != nullptr);
            value_ = typed_parse_result->value;
            values_.push_back(value_);
            if(external_value_ptr.has_value()) {
                T* ptr = std::any_cast<T*>(*external_value_ptr);
                if(ptr != nullptr) {
                    *ptr = value_;
                }
            }
        }
        const T& value() const {
            return value_;
        }
        const T& values(size_t idx) const {
            return values_[idx];
        }
    private:
        std::vector<T> values_;
        T value_;
};


#endif
