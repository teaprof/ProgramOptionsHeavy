#ifndef __BACKEND_OPTION_H__
#define __BACKEND_OPTION_H__

#include <memory>
#include <vector>
#include <optional>

class AbstractOption;
class AbstractNamedOption;
class AbstractPositionalOption;
class Alternatives;
class Compatibles;


class AbstractOptionVisitor {
    public:
        virtual void visit(std::shared_ptr<AbstractOption>) = 0;
        virtual void visit(std::shared_ptr<AbstractNamedOption>) = 0;
        virtual void visit(std::shared_ptr<AbstractPositionalOption>) = 0;
        virtual void visit(std::shared_ptr<Compatibles>) = 0;
        virtual void visit(std::shared_ptr<Alternatives>) = 0;
};

class AbstractOption : public std::enable_shared_from_this<AbstractOption> {
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
        AbstractNamedOption(const std::string& long_name) : long_name_{long_name} {}
        AbstractNamedOption(const std::string& long_name, const std::string& short_name) : AbstractOption(false), long_name_{long_name}, short_name_{short_name} {}

        void setValueRegex(const std::string& regex);
        const std::optional<std::string>& valueRegex() const;

        void setDefaultValue(const std::string& value);
        const std::optional<std::string>& defaultValue() const;

        const std::optional<std::string>& longName() const;
        const std::optional<std::string>& shortName() const;

        void accept(AbstractOptionVisitor& visitor) override;
    private:
        std::optional<std::string> long_name_;
        std::optional<std::string> short_name_;
        std::optional<std::string> value_regex_;
        std::optional<std::string> default_value_;
};

class AbstractPositionalOption : public AbstractOption {
    public:
        void accept(AbstractOptionVisitor& visitor) override;
};

class Compatibles : public AbstractOption {
    public:
        void accept(AbstractOptionVisitor& visitor) override;
};

class Alternatives : public AbstractOption {
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

inline void AbstractNamedOption::setValueRegex(const std::string& regex) {
    value_regex_ = regex;
}

inline const std::optional<std::string>& AbstractNamedOption::valueRegex() const {
    return value_regex_;
}

inline void AbstractNamedOption::setDefaultValue(const std::string& value) {
    default_value_ = value;
}

inline const std::optional<std::string>& AbstractNamedOption::defaultValue() const {
    return default_value_;
}

inline const std::optional<std::string>& AbstractNamedOption::longName() const {
    return long_name_;
}
inline const std::optional<std::string>& AbstractNamedOption::shortName() const {
    return short_name_;
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
inline void AbstractPositionalOption::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<AbstractPositionalOption>(shared_from_this()));
}
inline void Compatibles::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<Compatibles>(shared_from_this()));
}

inline void Alternatives::accept(AbstractOptionVisitor& visitor) {
    visitor.visit(std::static_pointer_cast<Alternatives>(shared_from_this()));
}

#endif