#ifndef __BACKEND_VALUE_SEMANTICS__
#define __BACKEND_VALUE_SEMANTICS__

#include "Exceptions.h"
#include <optional>
#include <string>

class BaseValueSemantics {
    public:
        virtual void setValue(const std::string& value) = 0;
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
        void setValue(const std::string& value) override {
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
            value_ = res;
        }
        int value() {
            return value_;
        }
    private:
        std::optional<int> min_, max_;
        int value_;
};

template<> 
class ValueSemantics<std::string> : public BaseValueSemantics {
    public:
        void setValue(const std::string& value) override {
            value_ = value;
        }
        const std::string& value() {
            return value_;
        }
    private:    
        std::string value_;
};
#endif