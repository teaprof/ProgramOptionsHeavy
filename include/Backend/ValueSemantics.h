#ifndef __BACKEND_VALUE_SEMANTICS__
#define __BACKEND_VALUE_SEMANTICS__

#include "Exceptions.h"
#include <any>
#include <optional>
#include <string>
#include <regex>
#include <concepts>
#include <type_traits>
#include <utility>

struct SemanticParseResult {
};

template<class T>
struct TypedSemanticParseResult : public SemanticParseResult {
    T value;
    TypedSemanticParseResult(const T& v) : value{v} {}
    void store(T& external_storage) {
        external_storage = value;
    }
};

class BaseValueSemantics {
    public:
        virtual std::shared_ptr<SemanticParseResult> semanticParse(const std::string& value) = 0; // TODO replace shared_ptr with something located on the stack
        virtual std::shared_ptr<SemanticParseResult> defaultValue() { return nullptr; }; // TODO replace shared_ptr with something located on the stack
        virtual void storeTo(std::shared_ptr<SemanticParseResult> value, std::any valueptr) = 0;
};

template<class T>
class TypedValueSemantics : public BaseValueSemantics {
    public:
        void storeTo(std::shared_ptr<SemanticParseResult> value, std::any valueptr) override {
            auto value_typed = std::static_pointer_cast<TypedSemanticParseResult<T>>(value);
            if(valueptr.has_value()) {
                T* value_typed_ptr =  std::any_cast<T*>(valueptr);
                value_typed->store(*value_typed_ptr);
            }
        }
        std::shared_ptr<SemanticParseResult> defaultValue() override {
            if(!default_value_.has_value())
                return nullptr;
            return std::make_shared<TypedSemanticParseResult<T>>(default_value_.value());
        }
        void setDefaultValue(const T& v) {
            default_value_ = v;
        }
        bool hasDefaultValue() {
            return default_value_.has_value();
        }
    private:
        std::optional<T> default_value_;
};

inline const std::string trim(const std::string& src) { // TODO: move to details or use boost::spirit instead of trim
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

template<std::integral IntType>
class ValueSemantics<IntType> : public TypedValueSemantics<IntType> {
    public:
        void setMinMax(IntType min, IntType max) {
            min_ = min;
            max_ = max;
        }
        void setMin(IntType min) {
            min_ = min;
        }
        void setMax(IntType max) {
            max_ = max;
        }
        std::shared_ptr<SemanticParseResult> semanticParse(const std::string& value) override {
            const std::string& trimmed = trim(value);
            size_t pos;
            IntType res;
            try {
                if constexpr(std::is_unsigned<IntType>::value) {
                    unsigned long long tmp = stoll(trimmed, &pos);
                    if(std::cmp_less(tmp, std::numeric_limits<IntType>::min()) || std::cmp_greater(tmp, std::numeric_limits<IntType>::max())) {
                        throw ValueIsOutOfRange(nullptr, value, "min..max");                        
                    };
                    res = tmp;
                } else {
                    signed long long tmp = stoll(trimmed, &pos);
                    if(std::cmp_less(tmp, std::numeric_limits<IntType>::min()) || std::cmp_greater(tmp, std::numeric_limits<IntType>::max())) {
                        throw ValueIsOutOfRange(nullptr, value, "min..max");                        
                    };
                    res = tmp;
                }
            } catch (std::invalid_argument) {
                throw InvalidValueType(nullptr, value, "int");
            } catch (std::out_of_range) {
                throw ValueIsOutOfRange(nullptr, value, "min..max");
            }
            if(pos != trimmed.length()) {
                throw InvalidValueType(nullptr, value, "int");
            }
            if(min_.has_value()) {
                if(res < min_.value()) {
                    throw ValueIsOutOfRange(nullptr, value, "min..max");
                }
            }
            if(max_.has_value()) {
                if(res > max_.value()) {
                    throw ValueIsOutOfRange(nullptr, value, "min..max");
                }
            }
            return std::make_shared<TypedSemanticParseResult<IntType>>(res);
        }        
    private:
        std::optional<IntType> min_, max_;
};

template<std::floating_point FloatType>
class ValueSemantics<FloatType> : public TypedValueSemantics<FloatType> {
    public:
        void setMinMax(FloatType min, FloatType max) {
            min_ = min;
            max_ = max;
        }
        void setMin(FloatType min) {
            min_ = min;
        }
        void setMax(FloatType max) {
            max_ = max;
        }
        std::shared_ptr<SemanticParseResult> semanticParse(const std::string& value) override {
            const std::string& trimmed = trim(value);
            size_t pos;
            FloatType res;
            try {
                auto tmp = stold(trimmed, &pos);
                if(tmp < std::numeric_limits<FloatType>::min() || tmp > std::numeric_limits<FloatType>::max()) {
                    throw ValueIsOutOfRange(nullptr, value, "min..max");                        
                };
                res = static_cast<FloatType>(tmp);
            } catch (std::invalid_argument) {
                throw InvalidValueType(nullptr, value, "int");
            } catch (std::out_of_range) {
                throw ValueIsOutOfRange(nullptr, value, "min..max");
            }
            if(pos != trimmed.length()) {
                throw InvalidValueType(nullptr, value, "int");
            }
            if(min_.has_value()) {
                if(res < min_.value()) {
                    throw ValueIsOutOfRange(nullptr, value, "min..max");
                }
            }
            if(max_.has_value()) {
                if(res > max_.value()) {
                    throw ValueIsOutOfRange(nullptr, value, "min..max");
                }
            }
            return std::make_shared<TypedSemanticParseResult<FloatType>>(res);
        }
    private:
        std::optional<FloatType> min_, max_;                
};

template<> 
class ValueSemantics<std::string> : public TypedValueSemantics<std::string> {
    public:
        std::shared_ptr<SemanticParseResult> semanticParse(const std::string& value) override {
            if(regex_.has_value()) {
                std::smatch m;
                if(!std::regex_match(value, m, *regex_)) {
                    assert(regex_hint_.has_value());
                    throw ValueMustMatchRegex(nullptr, *regex_hint_);
                }
            }
            return std::make_shared<TypedSemanticParseResult<std::string>>(value);
        }
        void setRegex(const std::regex& regex, const std::string& regex_hint) {
            regex_ = regex;
            regex_hint_ = regex_hint;
        }
    private:
        std::optional<std::regex> regex_;
        std::optional<std::string> regex_hint_;
};

template<> 
class ValueSemantics<bool> : public TypedValueSemantics<bool> {
    public:
        std::shared_ptr<SemanticParseResult> semanticParse(const std::string& value) override {
            const std::string& trimmed = trim(value);
            if(trimmed == "true" || trimmed == "TRUE" || trimmed == "True" || trimmed == "1") 
                return std::make_shared<TypedSemanticParseResult<bool>>(true);
            if(trimmed == "false" || trimmed == "FALSE" || trimmed == "False" || trimmed == "0") 
                return std::make_shared<TypedSemanticParseResult<bool>>(false);
            throw InvalidValueType(nullptr, value, "expected one of: TRUE, True, true, 1, FALSE, False, false, 0");
        }
};

#endif