#ifndef __BACKEND_OPTION_H__
#define __BACKEND_OPTION_H__

#include "ValueSemantics.h"
#include <memory>
#include <vector>
#include <optional>
#include <cassert>
#include <variant>
#include <functional>

class AbstractOption;
class AbstractNamedOption;
class AAbstractNamedOptionWithValue; // TODO rename
class AbstractNamedCommand;
class AbstractPositionalOption;
class Alternatives;
class OptionsGroup;


class AbstractOptionVisitor {
    public:
        virtual ~AbstractOptionVisitor() {};
        virtual void visit(std::shared_ptr<AbstractOption>) = 0;
        virtual void visit(std::shared_ptr<AbstractNamedOption>) = 0;
        virtual void visit(std::shared_ptr<AAbstractNamedOptionWithValue>) = 0;
        virtual void visit(std::shared_ptr<AbstractNamedCommand>) = 0;
        virtual void visit(std::shared_ptr<AbstractPositionalOption>) = 0;
        virtual void visit(std::shared_ptr<OptionsGroup>) = 0;
        virtual void visit(std::shared_ptr<Alternatives>) = 0;
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
        virtual void accept(AbstractOptionVisitor& visitor);

        bool required();
        void setRequired(bool val);        
    private:
        bool required_{false};
};

class AbstractNamedOption : public AbstractOption {
    public:
        AbstractNamedOption() {}
        AbstractNamedOption(const std::string& undecorated_long_name);
        AbstractNamedOption(const std::string& undecorated_long_name, const std::string& undecorated_short_name);        

        const std::optional<std::string>& longName() const;
        const std::optional<std::string>& shortName() const;
        const std::string displayName() const;

        void setMultiplicity(bool multiplicity); // TODO: rename to setMaxOccuranceCount which can be inf
        bool multiplicity() const;

        void accept(AbstractOptionVisitor& visitor) override;
    private:
        void sanitizeNames();
        std::optional<std::string> undecorated_long_name_; // long name without leading "--"
        std::optional<std::string> undecorated_short_name_; // short name without leading "-"
        bool multiplicity_{false};
};

class AAbstractNamedOptionWithValue : public AbstractNamedOption {
    public:
        AAbstractNamedOptionWithValue() {}
        AAbstractNamedOptionWithValue(const std::string& undecorated_long_name) : AbstractNamedOption(undecorated_long_name) {};
        AAbstractNamedOptionWithValue(const std::string& undecorated_long_name, const std::string& undecorated_short_name): AbstractNamedOption(undecorated_long_name, undecorated_short_name) {};   

        bool valueRequired() {
            return true;
        }
        void setValue(const std::string& str) {
            //TODO Remove this function
        }
};

template<class T>
class AbstractNamedOptionWithValue : public AAbstractNamedOptionWithValue {
    public:
        AbstractNamedOptionWithValue() {}
        AbstractNamedOptionWithValue(const std::string& undecorated_long_name) : AAbstractNamedOptionWithValue(undecorated_long_name) {};
        AbstractNamedOptionWithValue(const std::string& undecorated_long_name, const std::string& undecorated_short_name): AAbstractNamedOptionWithValue(undecorated_long_name, undecorated_short_name) {};   
        
        void accept(AbstractOptionVisitor& visitor) override;

        ValueSemantics<T>& valueSemantics() {
            return value_semantics_;
        }
        const ValueSemantics<T>& valueSemantics() const {
            return value_semantics_;
        }
    private:
        ValueSemantics<T> value_semantics_;
};


class AbstractNamedCommand : public AbstractNamedOption {
    /// \todo: this should be a separate class without support of values and short names
    public:          
        AbstractNamedCommand(const std::string long_name) : AbstractNamedOption(long_name) {};
        void accept(AbstractOptionVisitor& visitor) override;
};

class AbstractPositionalOption : public AbstractOption {
    public:
        void accept(AbstractOptionVisitor& visitor) override;
};

class OptionsGroup : public AbstractOption {
    public:
        void accept(AbstractOptionVisitor& visitor) override;
};

class Alternatives : public AbstractOption { // \todo: rename to OneOfOptions
    public:
        std::vector<std::shared_ptr<AbstractOption>> alternatives;
        Alternatives() {}
        Alternatives(std::shared_ptr<AbstractOption> alt1, std::shared_ptr<AbstractOption> alt2);
        Alternatives(std::shared_ptr<AbstractOption> alt1, std::shared_ptr<AbstractOption> alt2, std::shared_ptr<AbstractOption> alt3);

        std::shared_ptr<AbstractOption> addAlternative(std::shared_ptr<AbstractOption> opt);
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

inline AbstractNamedOption::AbstractNamedOption(const std::string& undecorated_long_name) : undecorated_long_name_{undecorated_long_name} {
    sanitizeNames();
}
inline AbstractNamedOption::AbstractNamedOption(const std::string& undecorated_long_name, const std::string& undecorated_short_name) : AbstractOption(false), undecorated_long_name_{undecorated_long_name}, undecorated_short_name_{undecorated_short_name} {
    sanitizeNames();
}

inline void AbstractNamedOption::sanitizeNames() {
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


inline void AbstractNamedOption::setMultiplicity(bool multiplicity) {
    multiplicity_ = multiplicity;
}

inline bool AbstractNamedOption::multiplicity() const {
    return multiplicity_;
}

inline const std::optional<std::string>& AbstractNamedOption::longName() const {
    return undecorated_long_name_;
}
inline const std::optional<std::string>& AbstractNamedOption::shortName() const {
    return undecorated_short_name_;
}
inline const std::string AbstractNamedOption::displayName() const {
    if(undecorated_long_name_.has_value())
        return std::string("--") + undecorated_long_name_.value();
    assert(undecorated_short_name_.has_value());
    return std::string("-") + undecorated_long_name_.value();
}

inline Alternatives::Alternatives(std::shared_ptr<AbstractOption> alt1, std::shared_ptr<AbstractOption> alt2) {
    alternatives.push_back(alt1);
    alternatives.push_back(alt2);
}

inline Alternatives::Alternatives(std::shared_ptr<AbstractOption> alt1, std::shared_ptr<AbstractOption> alt2, std::shared_ptr<AbstractOption> alt3) {
    alternatives.push_back(alt1);
    alternatives.push_back(alt2);
    alternatives.push_back(alt3);
}

inline std::shared_ptr<AbstractOption> Alternatives::addAlternative(std::shared_ptr<AbstractOption> opt) {
    alternatives.push_back(opt);
    return shared_from_this();
}


inline void AbstractOption::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(shared_from_this());
}
inline void AbstractNamedOption::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<AbstractNamedOption>(shared_from_this()));
}
inline void AbstractNamedCommand::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<AbstractNamedCommand>(shared_from_this()));
}
inline void AbstractPositionalOption::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<AbstractPositionalOption>(shared_from_this()));
}
inline void OptionsGroup::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<OptionsGroup>(shared_from_this()));
}

inline void Alternatives::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<Alternatives>(shared_from_this()));
}

#endif