#ifndef BACKEND_OPTION_H
#define BACKEND_OPTION_H

#include <cassert>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

class AbstractOption;
class LiteralString;
class NamedOption;
class AbstractNamedOptionWithValue;
class AbstractPositionalOption;
class AbstractPositionalOptionWithValue;
class OneOfPositional;
class OneOfNamed;
class OptionsGroup2;  // TODO rename

class BaseValueSemantics;
template <class T>
class ValueSemantics;

class AbstractOptionVisitor {
   public:
    virtual ~AbstractOptionVisitor(){};
    virtual void visit(std::shared_ptr<AbstractOption>) = 0;
    virtual void visit(std::shared_ptr<AbstractPositionalOption>) = 0;
    virtual void visit(std::shared_ptr<NamedOption>) = 0;
    virtual void visit(std::shared_ptr<LiteralString>) = 0;
    virtual void visit(std::shared_ptr<AbstractNamedOptionWithValue>) = 0;
    virtual void visit(std::shared_ptr<AbstractPositionalOptionWithValue>) = 0;
    virtual void visit(std::shared_ptr<OptionsGroup2>) = 0;
    virtual void visit(std::shared_ptr<OneOfPositional>) = 0;
    virtual void visit(std::shared_ptr<OneOfNamed>) = 0;
};

class AbstractOption : public std::enable_shared_from_this<AbstractOption> {
   public:
    // these objects should not be build directly, use std::make_shared to
    // create an instance
    AbstractOption();
    AbstractOption(bool required);

    std::vector<std::shared_ptr<AbstractOption>> unlocks() const;
    size_t unlocksCount() const;
    std::shared_ptr<AbstractOption> addUnlock(std::shared_ptr<AbstractOption> opt);

    bool required() const;
    void setRequired(bool val);

    void setMaxOccurreneCount(size_t max_count = std::numeric_limits<size_t>::max());
    size_t maxOccurrence() const;

    virtual void accept(AbstractOptionVisitor& visitor);

   private:
    bool required_{false};
    size_t max_occurence_{1};
    std::vector<std::shared_ptr<AbstractOption>> unlocks_;
};

class NamedOption : public AbstractOption {  // rename, should it be AbstractNamedOption??
   public:
    NamedOption() {}
    NamedOption(const std::string& undecorated_long_name);
    NamedOption(const std::string& undecorated_long_name, const std::string& undecorated_short_name);

    // at least longName() or shortName() should have value
    // leading dashes are removed
    const std::optional<std::string>& longName() const;
    const std::optional<std::string>& shortName() const;
    std::string displayName() const;

    void accept(AbstractOptionVisitor& visitor) override;

   private:
    void sanitizeNames();
    std::optional<std::string> undecorated_long_name_;   // long name without leading "--"
    std::optional<std::string> undecorated_short_name_;  // short name without leading "-"
};

class AbstractOptionWithValue {
   public:
    virtual ~AbstractOptionWithValue() {}
    virtual const BaseValueSemantics& baseValueSemantics() const = 0;
    virtual BaseValueSemantics& baseValueSemantics() = 0;

    // The number of values that the option can accept could be
    // - exactly N values;
    // - up to N values;
    // - infinite number of values (nValues doesn't matter in this case)
    enum class NValuesRole {
        EXACT,
        UPTO,      // todo: 0..upto or 1..upto
        INFINITE,  // todo: 0..inf or 1..inf
    };
    bool valueRequired() const { return value_required_; }
    bool setValueRequired(bool value_required) { return value_required_ = value_required; }
    void setNValues(NValuesRole role, size_t count = 1) {
        nvalues_role_ = role;
        nvalues_ = count;
    }
    NValuesRole nValuesRole() const { return nvalues_role_; }
    size_t nValues() const { return nvalues_; }

   private:
    NValuesRole nvalues_role_{NValuesRole::EXACT};
    size_t nvalues_{1};
    bool value_required_{false};
};

class AbstractPositionalOption : public AbstractOption {
   public:
    void accept(AbstractOptionVisitor& visitor) override;
};

class LiteralString : public AbstractPositionalOption {
   public:
    LiteralString(const std::string& str) : str_{str} {}

    const std::string& str() const;
    void accept(AbstractOptionVisitor& visitor) override;

   private:
    std::string str_;
};

class AbstractNamedOptionWithValue : public NamedOption, public AbstractOptionWithValue {
   public:
    AbstractNamedOptionWithValue() {}
    AbstractNamedOptionWithValue(const std::string& undecorated_long_name) : NamedOption(undecorated_long_name){};
    AbstractNamedOptionWithValue(const std::string& undecorated_long_name, const std::string& undecorated_short_name)
        : NamedOption(undecorated_long_name, undecorated_short_name) {}

    void accept(AbstractOptionVisitor& visitor) override;
};

template <class T>
class OptionWithValue {
   public:
    ValueSemantics<T>& valueSemantics() { return value_semantics_; }
    const ValueSemantics<T>& valueSemantics() const { return value_semantics_; }

   private:
    ValueSemantics<T> value_semantics_;
};

template <class T>
class NamedOptionWithValue : public AbstractNamedOptionWithValue, public OptionWithValue<T> {
   public:
    NamedOptionWithValue() {}
    NamedOptionWithValue(const std::string& undecorated_long_name) : AbstractNamedOptionWithValue(undecorated_long_name){};
    NamedOptionWithValue(const std::string& undecorated_long_name, const std::string& undecorated_short_name)
        : AbstractNamedOptionWithValue(undecorated_long_name, undecorated_short_name){};

    const BaseValueSemantics& baseValueSemantics() const override { return OptionWithValue<T>::valueSemantics(); }
    BaseValueSemantics& baseValueSemantics() override { return OptionWithValue<T>::valueSemantics(); }
};

class AbstractPositionalOptionWithValue : public AbstractPositionalOption, public AbstractOptionWithValue {
   public:
    // TODO: How multiple occurences can be distingueshed from multiple values
    // in one occurence?
    void accept(AbstractOptionVisitor& visitor) override;
};

template <class T>
class PositionalOptionWithValue : public AbstractPositionalOptionWithValue, public OptionWithValue<T> {
   public:
    const BaseValueSemantics& baseValueSemantics() const override { return OptionWithValue<T>::valueSemantics(); }
    BaseValueSemantics& baseValueSemantics() override { return OptionWithValue<T>::valueSemantics(); }
};

class OptionsGroup2 : public AbstractOption {
    /// TODO what does `required_` mean in this case?
   public:
    void accept(AbstractOptionVisitor& visitor) override;
};

class OneOfPositional : public AbstractPositionalOption {
   public:
    std::vector<std::shared_ptr<AbstractPositionalOption>> alternatives;
    OneOfPositional() {}
    OneOfPositional(std::shared_ptr<AbstractPositionalOption> alt1, std::shared_ptr<AbstractPositionalOption> alt2);
    OneOfPositional(std::shared_ptr<AbstractPositionalOption> alt1,
                    std::shared_ptr<AbstractPositionalOption> alt2,
                    std::shared_ptr<AbstractPositionalOption> alt3);

    std::shared_ptr<AbstractPositionalOption> alternative(size_t idx) { return alternatives[idx]; }
    size_t alternativesSize() { return alternatives.size(); }

    std::shared_ptr<OneOfPositional> addAlternative(std::shared_ptr<AbstractPositionalOption> opt);
    std::shared_ptr<OneOfPositional> addAlternative2(std::shared_ptr<AbstractOption> opt);
    void accept(AbstractOptionVisitor& visitor);
};

class OneOfNamed : public NamedOption {
   public:
    std::vector<std::shared_ptr<NamedOption>> alternatives;
    OneOfNamed() {}
    OneOfNamed(std::shared_ptr<NamedOption> alt1, std::shared_ptr<NamedOption> alt2);
    OneOfNamed(std::shared_ptr<NamedOption> alt1, std::shared_ptr<NamedOption> alt2, std::shared_ptr<NamedOption> alt3);

    std::shared_ptr<NamedOption> alternative(size_t idx) { return alternatives[idx]; }
    size_t alternativesSize() { return alternatives.size(); }

    std::shared_ptr<OneOfNamed> addAlternative(std::shared_ptr<NamedOption> opt);
    std::shared_ptr<OneOfNamed> addAlternative2(std::shared_ptr<AbstractOption> opt);
    void accept(AbstractOptionVisitor& visitor);
};

inline AbstractOption::AbstractOption() : required_{false} {
    // nothing to do
}
inline AbstractOption::AbstractOption(bool required) : required_{required} {
    // nothing to do
}

inline std::vector<std::shared_ptr<AbstractOption>> AbstractOption::unlocks() const {
    return unlocks_;
}
inline size_t AbstractOption::unlocksCount() const {
    return unlocks_.size();
}

inline std::shared_ptr<AbstractOption> AbstractOption::addUnlock(std::shared_ptr<AbstractOption> opt) {
    unlocks_.push_back(opt);
    return shared_from_this();
}

inline bool AbstractOption::required() const {
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

inline const std::string& LiteralString::str() const {
    return str_;
}

inline NamedOption::NamedOption(const std::string& undecorated_long_name) : undecorated_long_name_{undecorated_long_name} {
    sanitizeNames();
}
inline NamedOption::NamedOption(const std::string& undecorated_long_name, const std::string& undecorated_short_name)
    : AbstractOption(false), undecorated_long_name_{undecorated_long_name}, undecorated_short_name_{undecorated_short_name} {
    sanitizeNames();
}

inline void NamedOption::sanitizeNames() {
    if (undecorated_long_name_.has_value()) {
        if (undecorated_long_name_->starts_with("--")) {
            undecorated_long_name_->erase(0, 2);
        };
        assert(!undecorated_long_name_->empty() && undecorated_long_name_.value()[0] != '-');
    };
    if (undecorated_short_name_.has_value()) {
        if (undecorated_short_name_->starts_with("-")) {
            undecorated_short_name_->erase(0, 1);
        };
        assert(!undecorated_short_name_->empty() && undecorated_short_name_.value()[0] != '-');
    };
}

inline const std::optional<std::string>& NamedOption::longName() const {
    return undecorated_long_name_;
}
inline const std::optional<std::string>& NamedOption::shortName() const {
    return undecorated_short_name_;
}
inline std::string NamedOption::displayName() const {  // TODO where is it used?
    if (undecorated_long_name_.has_value()) {
        return std::string("--") + undecorated_long_name_.value();
    }
    assert(undecorated_short_name_.has_value());
    return std::string("-") + undecorated_long_name_.value();
}

inline OneOfPositional::OneOfPositional(std::shared_ptr<AbstractPositionalOption> alt1,
                                        std::shared_ptr<AbstractPositionalOption> alt2) {
    alternatives.push_back(alt1);
    alternatives.push_back(alt2);
}

inline OneOfPositional::OneOfPositional(std::shared_ptr<AbstractPositionalOption> alt1,
                                        std::shared_ptr<AbstractPositionalOption> alt2,
                                        std::shared_ptr<AbstractPositionalOption> alt3) {
    alternatives.push_back(alt1);
    alternatives.push_back(alt2);
    alternatives.push_back(alt3);
}

inline std::shared_ptr<OneOfPositional> OneOfPositional::addAlternative(std::shared_ptr<AbstractPositionalOption> opt) {
    alternatives.push_back(opt);
    return std::static_pointer_cast<OneOfPositional>(shared_from_this());
}
inline std::shared_ptr<OneOfPositional> OneOfPositional::addAlternative2(std::shared_ptr<AbstractOption> opt) {
    auto p = std::dynamic_pointer_cast<AbstractPositionalOption>(opt);
    assert(p != nullptr);
    alternatives.push_back(p);
    return std::static_pointer_cast<OneOfPositional>(shared_from_this());
}

inline OneOfNamed::OneOfNamed(std::shared_ptr<NamedOption> alt1, std::shared_ptr<NamedOption> alt2) {
    alternatives.push_back(alt1);
    alternatives.push_back(alt2);
}

inline OneOfNamed::OneOfNamed(std::shared_ptr<NamedOption> alt1,
                              std::shared_ptr<NamedOption> alt2,
                              std::shared_ptr<NamedOption> alt3) {
    alternatives.push_back(alt1);
    alternatives.push_back(alt2);
    alternatives.push_back(alt3);
}

inline std::shared_ptr<OneOfNamed> OneOfNamed::addAlternative(std::shared_ptr<NamedOption> opt) {
    alternatives.push_back(opt);
    return std::static_pointer_cast<OneOfNamed>(shared_from_this());
}
inline std::shared_ptr<OneOfNamed> OneOfNamed::addAlternative2(std::shared_ptr<AbstractOption> opt) {
    auto p = std::dynamic_pointer_cast<NamedOption>(opt);
    assert(p != nullptr);
    alternatives.push_back(p);
    return std::static_pointer_cast<OneOfNamed>(shared_from_this());
}

inline void AbstractOption::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(shared_from_this());
}
inline void LiteralString::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::dynamic_pointer_cast<LiteralString>(shared_from_this()));
}

inline void NamedOption::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::dynamic_pointer_cast<NamedOption>(shared_from_this()));
}
inline void AbstractNamedOptionWithValue::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::dynamic_pointer_cast<AbstractNamedOptionWithValue>(shared_from_this()));
}
inline void AbstractPositionalOption::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::dynamic_pointer_cast<AbstractPositionalOption>(shared_from_this()));
}
inline void AbstractPositionalOptionWithValue::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::dynamic_pointer_cast<AbstractPositionalOptionWithValue>(shared_from_this()));
}
inline void OptionsGroup2::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::dynamic_pointer_cast<OptionsGroup2>(shared_from_this()));
}

/*inline void NamedCommand::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<NamedCommand>(shared_from_this()));
}*/

inline void OneOfPositional::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<OneOfPositional>(shared_from_this()));
}

inline void OneOfNamed::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<OneOfNamed>(shared_from_this()));
}

inline bool isPositional(std::shared_ptr<AbstractOption> opt) {  /// todo: 2del
    return std::dynamic_pointer_cast<AbstractPositionalOption>(opt) != nullptr ||
           std::dynamic_pointer_cast<LiteralString>(opt) != nullptr;
}

#endif
