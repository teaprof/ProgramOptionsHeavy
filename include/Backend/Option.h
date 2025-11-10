#ifndef __BACKEND_OPTION_H__
#define __BACKEND_OPTION_H__

#include <memory>
#include <vector>
#include <optional>
#include <cassert>
#include <variant>
#include <functional>
#include <limits>

class AbstractOption;
class NamedOption;
class AbstractNamedOptionWithValue;
class AbstractPositionalOption;
class NamedCommand;
class OneOf;
class OptionsGroup;

class BaseValueSemantics;
template<class T> class ValueSemantics;


class AbstractOptionVisitor {
    public:
        virtual ~AbstractOptionVisitor() {};
        virtual void visit(std::shared_ptr<AbstractOption>) = 0;
        virtual void visit(std::shared_ptr<NamedOption>) = 0;
        virtual void visit(std::shared_ptr<AbstractNamedOptionWithValue>) = 0;
        virtual void visit(std::shared_ptr<NamedCommand>) = 0;
        virtual void visit(std::shared_ptr<AbstractPositionalOption>) = 0;
        virtual void visit(std::shared_ptr<OptionsGroup>) = 0;
        virtual void visit(std::shared_ptr<OneOf>) = 0;
};

class AbstractOption : public std::enable_shared_from_this<AbstractOption> {
    /// \todo: make constructor to be private
    /*struct Private {
        explicit Private() = default;
    };*/
    public:
        AbstractOption() : required_{false} {}
        AbstractOption(bool required) : required_{required} {}
        std::vector<std::shared_ptr<AbstractOption>> unlocks;
        std::shared_ptr<AbstractOption> addUnlock(std::shared_ptr<AbstractOption> opt);

        bool required();
        void setRequired(bool val);

        void setMaxOccurreneCount(size_t max_count = std::numeric_limits<size_t>::max());
        size_t maxOccurrence() const;

        virtual void accept(AbstractOptionVisitor& visitor);
    private:
        bool required_{false};
        size_t max_occurence_{1};
};

class NamedOption : public AbstractOption {
    public:
        NamedOption() {}
        NamedOption(const std::string& undecorated_long_name);
        NamedOption(const std::string& undecorated_long_name, const std::string& undecorated_short_name);        

        const std::optional<std::string>& longName() const;
        const std::optional<std::string>& shortName() const;
        const std::string displayName() const;

        void accept(AbstractOptionVisitor& visitor) override;
    private:
        void sanitizeNames();
        std::optional<std::string> undecorated_long_name_; // long name without leading "--"
        std::optional<std::string> undecorated_short_name_; // short name without leading "-"
};

class AbstractOptionWithValue {
    public:
        virtual const BaseValueSemantics& baseValueSemantics() const = 0;
        virtual BaseValueSemantics& baseValueSemantics() = 0;
};

class AbstractNamedOptionWithValue : public NamedOption, public AbstractOptionWithValue {
    public:
        AbstractNamedOptionWithValue() {}
        AbstractNamedOptionWithValue(const std::string& undecorated_long_name) : NamedOption(undecorated_long_name) {};
        AbstractNamedOptionWithValue(const std::string& undecorated_long_name, const std::string& undecorated_short_name): NamedOption(undecorated_long_name, undecorated_short_name) {}

        bool valueRequired() {
            return true;
        }
        void accept(AbstractOptionVisitor& visitor) override;
};

template<class T> /// TODO actually this class can be non-template (type-agnostic)
class NamedOptionWithValue : public AbstractNamedOptionWithValue {
    public:
        NamedOptionWithValue() {}
        NamedOptionWithValue(const std::string& undecorated_long_name) : AbstractNamedOptionWithValue(undecorated_long_name) {};
        NamedOptionWithValue(const std::string& undecorated_long_name, const std::string& undecorated_short_name): AbstractNamedOptionWithValue(undecorated_long_name, undecorated_short_name) {};   
        
        const BaseValueSemantics& baseValueSemantics() const override {
            return valueSemantics();
        }
        BaseValueSemantics& baseValueSemantics() override {
            return valueSemantics();
        }

        ValueSemantics<T>& valueSemantics() {
            return value_semantics_;
        }
        const ValueSemantics<T>& valueSemantics() const {
            return value_semantics_;
        }
        void accept(AbstractOptionVisitor& visitor) override;
    private:
        ValueSemantics<T> value_semantics_;
};


class NamedCommand : public NamedOption {
    /// \todo: this should be a separate class without support of values and short names
    /// TODO remove this class?
    public:          
        NamedCommand(const std::string long_name) : NamedOption(long_name) {};
        void accept(AbstractOptionVisitor& visitor) override;
};

class AbstractPositionalOption : public AbstractOption, public AbstractOptionWithValue {
    public:
        void accept(AbstractOptionVisitor& visitor) override;
};

template<class T>
class PositionalOption : public AbstractPositionalOption {
    public:
        const BaseValueSemantics& baseValueSemantics() const override {
            return valueSemantics();
        }
        BaseValueSemantics& baseValueSemantics() override {
            return valueSemantics();
        }
        ValueSemantics<T>& valueSemantics() {
            return value_semantics_;
        }
        const ValueSemantics<T>& valueSemantics() const {
            return value_semantics_;
        }
    private:
        ValueSemantics<T> value_semantics_;
};

class OptionsGroup : public AbstractOption {
    /// TODO what does required_ mean in this case?
    public:
        void accept(AbstractOptionVisitor& visitor) override;        
};

class OneOf : public AbstractOption {
    public:
        std::vector<std::shared_ptr<AbstractOption>> alternatives;
        OneOf() {}
        OneOf(std::shared_ptr<AbstractOption> alt1, std::shared_ptr<AbstractOption> alt2);
        OneOf(std::shared_ptr<AbstractOption> alt1, std::shared_ptr<AbstractOption> alt2, std::shared_ptr<AbstractOption> alt3);

        std::shared_ptr<OneOf> addAlternative(std::shared_ptr<AbstractOption> opt);
        void accept(AbstractOptionVisitor& visitor) override;
};

inline std::shared_ptr<AbstractOption> AbstractOption::addUnlock(std::shared_ptr<AbstractOption> opt) {
    unlocks.push_back(opt);
    return shared_from_this();
}

inline bool AbstractOption::required() {
    return required_;
}

inline void AbstractOption::setRequired(bool val) {
    required_ = val;
}
inline void AbstractOption::setMaxOccurreneCount(size_t max_count) {
    max_occurence_ = max_count;
}
inline size_t AbstractOption::maxOccurrence() const {
    return max_occurence_;
}

inline NamedOption::NamedOption(const std::string& undecorated_long_name) : undecorated_long_name_{undecorated_long_name} {
    sanitizeNames();
}
inline NamedOption::NamedOption(const std::string& undecorated_long_name, const std::string& undecorated_short_name) : AbstractOption(false), undecorated_long_name_{undecorated_long_name}, undecorated_short_name_{undecorated_short_name} {
    sanitizeNames();
}

inline void NamedOption::sanitizeNames() {
    if(undecorated_long_name_.has_value()) {
        if(undecorated_long_name_->starts_with("--")) {
            undecorated_long_name_->erase(0, 2);
        };
        assert(undecorated_long_name_->size() > 0 && undecorated_long_name_.value()[0] != '-');
    };
    if(undecorated_short_name_.has_value()) {
        if(undecorated_short_name_->starts_with("-")) {
            undecorated_short_name_->erase(0, 1);
        };
        assert(undecorated_short_name_->size() > 0 && undecorated_short_name_.value()[0] != '-');
    };
}

inline const std::optional<std::string>& NamedOption::longName() const {
    return undecorated_long_name_;
}
inline const std::optional<std::string>& NamedOption::shortName() const {
    return undecorated_short_name_;
}
inline const std::string NamedOption::displayName() const {
    if(undecorated_long_name_.has_value())
        return std::string("--") + undecorated_long_name_.value();
    assert(undecorated_short_name_.has_value());
    return std::string("-") + undecorated_long_name_.value();
}

inline OneOf::OneOf(std::shared_ptr<AbstractOption> alt1, std::shared_ptr<AbstractOption> alt2) {
    alternatives.push_back(alt1);
    alternatives.push_back(alt2);
}

inline OneOf::OneOf(std::shared_ptr<AbstractOption> alt1, std::shared_ptr<AbstractOption> alt2, std::shared_ptr<AbstractOption> alt3) {
    alternatives.push_back(alt1);
    alternatives.push_back(alt2);
    alternatives.push_back(alt3);
}

inline std::shared_ptr<OneOf> OneOf::addAlternative(std::shared_ptr<AbstractOption> opt) {
    alternatives.push_back(opt);
    return std::static_pointer_cast<OneOf>(shared_from_this());
}


inline void AbstractOption::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(shared_from_this());
}
inline void NamedOption::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<NamedOption>(shared_from_this()));
}
inline void AbstractNamedOptionWithValue::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<AbstractNamedOptionWithValue>(shared_from_this()));
}
template<class T>
inline void NamedOptionWithValue<T>::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<AbstractNamedOptionWithValue>(shared_from_this()));
}
inline void NamedCommand::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<NamedCommand>(shared_from_this()));
}
inline void AbstractPositionalOption::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<AbstractPositionalOption>(shared_from_this()));
}
inline void OptionsGroup::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<OptionsGroup>(shared_from_this()));
}

inline void OneOf::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<OneOf>(shared_from_this()));
}

#endif