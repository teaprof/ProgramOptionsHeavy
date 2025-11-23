#ifndef __BACKEND_VALUE_SEMANTICS__
#define __BACKEND_VALUE_SEMANTICS__

#include "Exceptions.h"
#include "SemanticParseResult.h"
#include <any>
#include <optional>
#include <string>
#include <regex>
#include <concepts>
#include <type_traits>
#include <utility>

class BaseValueStorage;

class BaseValueSemantics { // TODO rename to ValueParser and all variables of this type
    public:
        // TODO replace shared_ptr with something faster located on the stack
        virtual std::any semanticParse(const std::string& value) = 0;  // TODO rename to parseAndSetValue
        //virtual std::shared_ptr<BaseValueStorage> createStorage() = 0;
        // TODO add setImplicitValue

        virtual bool hasDefaultValue() = 0;
        virtual bool hasImplicitValue() = 0;
        virtual std::any setToDefault() = 0;
        virtual std::any setToImplicit() = 0;
        
        /// @brief throw or not an exception if parsed value is not in Unlocks
        void setOnlyAllowedValues(bool flag) {
            only_allowed_values_ = flag;
        }        

        virtual void store(const std::any& value, std::any& dest_any_pointer) = 0;

        virtual std::vector<std::shared_ptr<AbstractOption>> getUnlocks() = 0;
    protected:
        bool only_allowed_values_{false};
};

template<class T>
class TypedValueSemantics : public BaseValueSemantics {
    public:
        TypedValueSemantics() {}
        TypedValueSemantics(T& ref) : external_ref_{ref} {}
        /*std::shared_ptr<SemanticParseResult> defaultValue() override {
            if(!default_value_.has_value())
                return nullptr;
            return constructResult(default_value_.value());
        }*/
        void setExternalStorage(T& ref) {
            external_ref_ = ref;        
        }
        void setDefaultValue(const T& v) {
            default_value_ = v;
        }
        bool hasDefaultValue() override {
            return default_value_.has_value();
        }
        void setImplicitValue(const T& v) {
            implicit_value_ = v;
        }
        bool hasImplicitValue() override {
            return implicit_value_.has_value();
        }
        void setValue(const T& val) {
            checkIfValueIsInList(val);
            value_ = val;
            if(external_ref_.has_value()) {
                external_ref_.value().get() = val;
            }
        }
        void store(const std::any& value, std::any& dest_any_pointer) override {
            *std::any_cast<T*>(dest_any_pointer) = std::any_cast<T>(value);
        }
        std::any setToDefault() override {
            assert(default_value_.has_value());
            setValue(*default_value_);
            return *default_value_;
        }
        std::any setToImplicit() override {
            assert(implicit_value_.has_value());
            setValue(*implicit_value_);
            return *implicit_value_;
        }
        /// std::enable_if T is not floating point
        std::vector<std::shared_ptr<AbstractOption>>& unlocks(const T& value) {
            return unlocks_[value];
        }
        const std::vector<std::shared_ptr<AbstractOption>>& unlocks(const T& value) const {
            return unlocks_[value];
        }
        std::vector<std::shared_ptr<AbstractOption>> getUnlocks() override {
            assert(value_.has_value());
            return unlocks_[*value_];
        }
        const T& value() {
            assert(value_.has_value());
            return *value_;
        }
    protected:
        std::optional<T> value_;
        std::optional<std::reference_wrapper<T>> external_ref_;
    private:
        std::optional<T> default_value_;
        std::optional<T> implicit_value_;
        std::map<T, std::vector<std::shared_ptr<AbstractOption>>> unlocks_;

        void checkIfValueIsInList(const T& val) {
            if(only_allowed_values_ == false)
                return;
            if(unlocks_.contains(val) == false)
                throw InvalidOptionValue(nullptr, "", "");
        }
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
        std::any semanticParse(const std::string& value) override {
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
            TypedValueSemantics<IntType>::setValue(res);
            return res;
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
        std::any semanticParse(const std::string& value) override {
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
            TypedValueSemantics<FloatType>::setValue(res);
            return res;
        }
    private:
        std::optional<FloatType> min_, max_;                
};

template<> 
class ValueSemantics<std::string> : public TypedValueSemantics<std::string> {
    public:
        std::any semanticParse(const std::string& value) override {
            if(regex_.has_value()) {
                std::smatch m;
                if(!std::regex_match(value, m, *regex_)) {
                    assert(regex_hint_.has_value());
                    throw ValueMustMatchRegex(nullptr, *regex_hint_);
                }
            }
            TypedValueSemantics<std::string>::setValue(value);
            return value;
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
        std::any semanticParse(const std::string& value) override {
            const std::string& trimmed = trim(value);
            if(trimmed == "true" || trimmed == "TRUE" || trimmed == "True" || trimmed == "1")  {
                setValue(true);
                return true;
            };
            if(trimmed == "false" || trimmed == "FALSE" || trimmed == "False" || trimmed == "0") {
                setValue(false);
                return false;
            }
            throw InvalidValueType(nullptr, value, "expected one of: TRUE, True, true, 1, FALSE, False, false, 0");
        }
};

#endif