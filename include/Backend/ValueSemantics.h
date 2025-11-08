#ifndef __BACKEND_VALUE_SEMANTICS__
#define __BACKEND_VALUE_SEMANTICS__

#include <optional>
#include <string>

template<class T>
class ValueSemantics {
    public:
        bool check(const std::string& value);
        void setDefaultValue(const T& value);
        // TODO checker should check if default value is within [min, max]
};

template<> // TODO: any integral type
class ValueSemantics<int> {
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
        bool check(const std::string& value) {
            // TODO: trim value
            size_t pos;
            int res =  stoi(value, &pos);
            if(pos != value.length()) {
                return false;
            }
            if(min_.has_value()) {
                if(res < min_.value()) {
                    return false;
                }
            }
            if(max_.has_value()) {
                if(res > max_.value()) {
                    return false;
                }
            }
            return true;
        }
    private:
        std::optional<int> min_, max_;
};

template<> 
class ValueSemantics<std::string> {
    public:
        bool check(const std::string& value) {
            return true;
        }
};
#endif