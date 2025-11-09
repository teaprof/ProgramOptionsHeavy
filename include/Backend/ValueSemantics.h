#ifndef __BACKEND_VALUE_SEMANTICS__
#define __BACKEND_VALUE_SEMANTICS__

#include "Exceptions.h"
#include <any>
#include <optional>
#include <string>

class BaseValueSemantics {
    public:
        virtual void checkValue(const std::string& value) {
            setValue2(value, {});
        }
        virtual void setValue2(const std::string& value, std::any reference) = 0;  // TODO rename to decodeValue  
};

template<class T>
class ValueSemantics : public BaseValueSemantics {
    /*public:
        void setValue(const std::string& value) override {
            value_ = value;
        }
        void setDefaultValue(const T& value);        
    private:
        T& value_;*/
    
};

template<> // TODO: any integral type
class ValueSemantics<int> : public BaseValueSemantics {
    public:
        void setMinMax(int min, int max) {
            min_ = min;
            max_ = max;
        }
        void setMin(int min) {
            min_ = min;
        }
        void setMax(int max) {
            max_ = max;
        }
        void setValue2(const std::string& value, std::any valueptr) override {
            // TODO: trim value
            size_t pos;
            int res;
            try {
                res =  stoi(value, &pos);
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
            if(valueptr.has_value()) {
                int* p = std::any_cast<int*>(valueptr);
                if(p != nullptr)
                    *p = res;
            };
        }
    private:
        std::optional<int> min_, max_;
};

template<> 
class ValueSemantics<std::string> : public BaseValueSemantics {
    public:
        void setValue2(const std::string& value, std::any valueptr) override {
            value_ = value;
            std::string* p = std::any_cast<std::string*>(valueptr);
            if(p != nullptr)
                *p = value;
        }
        const std::string& value() {
            return value_;
        }
    private:    
        std::string value_;
};
#endif