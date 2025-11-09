#ifndef __BACKEND_VALUE_SEMANTICS__
#define __BACKEND_VALUE_SEMANTICS__

#include "Exceptions.h"
#include <any>
#include <optional>
#include <string>
#include <concepts>
#include <type_traits>

struct SemanticParseResult {
};

template<class IntType>
struct TypedSemanticParseResult : public SemanticParseResult {
    IntType value;
    TypedSemanticParseResult(const IntType& v) : value{v} {}
    void store(IntType& external_storage) {
        external_storage = value;
    }
};

class BaseValueSemantics {
    public:
        virtual std::shared_ptr<SemanticParseResult> semanticParse(const std::string& value) = 0; // TODO replace shared_ptr with something located on the stack
        virtual void setValue(std::shared_ptr<SemanticParseResult> value, std::any valueptr) = 0;
};

template<class IntType>
class TypedValueSemantics : public BaseValueSemantics {
    public:
        void setValue(std::shared_ptr<SemanticParseResult> value, std::any valueptr) override {
            auto value_typed = std::static_pointer_cast<TypedSemanticParseResult<IntType>>(value);
            if(valueptr.has_value()) {
                IntType* value_typed_ptr =  std::any_cast<IntType*>(valueptr);
                value_typed->store(*value_typed_ptr);
            }
        }
};

template<std::integral IntType> // TODO: any integral type
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
            // TODO: trim value
            size_t pos;
            IntType res;
            try {
                if constexpr(std::is_unsigned<IntType>::value) {
                    unsigned long long tmp = stoll(value, &pos);
                    if(tmp < std::numeric_limits<IntType>::min() || tmp > std::numeric_limits<IntType>::max()) { /// TODO check that type conversion work properly
                        throw ValueIsOutOfRange(nullptr, value, "min..max");                        
                    };
                    res = tmp;
                } else {
                    signed long long tmp = stoll(value, &pos);
                    if(tmp < std::numeric_limits<IntType>::min() || tmp > std::numeric_limits<IntType>::max()) {
                        throw ValueIsOutOfRange(nullptr, value, "min..max");                        
                    };
                    res = tmp;
                }
            } catch (std::invalid_argument) {
                throw InvalidValueType(nullptr, value, "int");
            } catch (std::out_of_range) {
                throw ValueIsOutOfRange(nullptr, value, "min..max");
            }
            if(pos != value.length()) {
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

template<> 
class ValueSemantics<std::string> : public TypedValueSemantics<std::string> {
    public:
        std::shared_ptr<SemanticParseResult> semanticParse(const std::string& value) override {
            return std::make_shared<TypedSemanticParseResult<std::string>>(value);
        }
};

// TODO ValueSemantics<bool>(TRUE, true, True, 1, FALSE, false, False, 0)
// TODO ValueSemantics<double>, <float>
#endif