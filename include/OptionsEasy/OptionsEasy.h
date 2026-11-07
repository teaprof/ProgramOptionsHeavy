#ifndef BACKEND_FACADE_H
#define BACKEND_FACADE_H

#include <Backend/Option.h>
#include <Backend/ValueSemantics.h>
#include <Backend/ValueStorage.h>
#include <Backend/Parser.h>
#include <Checker/Checker.h>
#include <Completer/Completer2.h>
#include <Help/HelpStringsStorage.h>
#include <utils/strutils.h>

namespace program_options_heavy {


/* TODO:
- [x] NamedOption : public AbstractOption  <- names
- [ ] class LiteralString : public AbstractPositionalOption <- name
- [x] template <class T> class NamedOptionWithValue : public AbstractNamedOptionWithValue, public OptionWithValue<T> <- T, names
- [x] template <class T> class PositionalOptionWithValue : public AbstractPositionalOptionWithValue, public OptionWithValue<T> <- T, display_name
- [ ] class OptionsGroup : public AbstractOption <- display_name
- [ ] class OneOfPositional : public AbstractPositionalOption  <- display_name
- [ ] class OneOfNamed : public NamedOption <- display_name

TODO: implement flags without values like --autocomplete (it doesn't expect a value)
*/

template<class T>
class ValueEasy {
    public:
        ValueEasy(std::reference_wrapper<T> storage) : storage_{storage} {}
        //ValueEasy(std::reference_wrapper<std::optional<T>> storage) : storage_{storage} {}

        void setImplicitValue(const T& val) { implicit_value_ = val; }
        void setDefaultValue(const T& val) { default_value_ = val; }
        void setNValues(AbstractOptionWithValue::NValuesRole role, size_t count = 1) { /* TODO: NOT IMPLEMENTED YET */  throw std::logic_error("not implemented"); }
    private:
        std::reference_wrapper<T> storage_;
        //using refT = std::reference_wrapper<T>;
        //using refOptT = std::reference_wrapper<std::optional<T>>;
        //std::variant<refT, refOptT> storage_;

        std::optional<T> implicit_value_;
        std::optional<T> default_value_;
        friend class OptionsEasy;
};

class OptionsEasy;

template<class T>
class OptionHelper {
    public:
        OptionHelper(std::shared_ptr<T> opt) : opt_{opt} {}
        void setMaxOccurrence(size_t count) { opt_->setMaxOccurrence(count); }
        void setRequired(bool val)  { opt_->setRequired(val); }
        std::string setDisplayName(const std::string& display_name) const  { opt_->setDisplayName(display_name); }
            
        T& operator*() { return opt_; }
    private:
        std::shared_ptr<T> opt_;        
};

class OptionsEasy {
   // TODO:  make display_name optional parameter. If it is not specified use OptionTypeName1, OptionTypeName2 and so on
   public:
    OptionsEasy() {
        root_ = std::make_shared<AbstractOption>("root");
    }
    template <class T>
    std::shared_ptr<NamedOptionWithValue<T>> makeNamedOption(const std::string& names,
                                                        const std::string& help_message) {
        auto res = makeOptionWithName<NamedOptionWithValue<T>>(names);        
        help_ << res << help_message;
        root_->addUnlock(res);
        return res;
    }

    template <class T>
    std::shared_ptr<NamedOptionWithValue<T>> makeNamedOptionWithValue(const std::string& names,
                                                        ValueEasy<T> storage, // TODO: value is not storage
                                                        const std::string& help_message) {
        auto res = makeOptionWithName<NamedOptionWithValue<T>>(names);
        if(storage.default_value_.has_value()) {
            res->valueSemantics().setDefaultValue(storage.default_value_.value());
        }
        if(storage.implicit_value_.has_value()) {
            res->valueSemantics().setImplicitValue(storage.implicit_value_.value());
        }
        /// TODO: simplify the following 
        /*if(std::holds_alternative<std::reference_wrapper<T>>(storage.storage_)) {
            res->valueSemantics().setExternalStorage(std::get<std::reference_wrapper<T>>(storage.storage_));
        } else {
            assert(std::holds_alternative<std::reference_wrapper<std::optional<T>>>(storage.storage_));
            res->valueSemantics().setExternalStorage(std::get<std::reference_wrapper<std::optional<T>>>(storage.storage_));
        } */
        res->valueSemantics().setExternalStorage(storage.storage_);
        help_ << res << help_message;
        root_->addUnlock(res);
        return res;
    }

    template <class T>
    std::shared_ptr<PositionalOptionWithValue<T>> makePositionalOptionWithValue(ValueEasy<T> storage, std::string& help_message) {
        auto res = std::make_shared<PositionalOptionWithValue<T>>("noname");
        res->valueSemantics().setExternalStorage(storage);
        help_ << res << help_message;
        root_->addUnlock(res);
        return res;
    }

    template <class T>
    std::shared_ptr<PositionalOptionWithValue<T>> makePositionalOptionWithValue(ValueEasy<T> storage,
                                                                       const std::string& help_message) {
        auto res = std::make_shared<PositionalOptionWithValue<T>>("noname");
        //res->valueSemantics().setExternalStorage(storage);
        //assert(max_occurrence == 1);
        //res->valueSemantics().setMaxOccurrence(max_occurrence); // inherited from boost::program_options
        help_ << res << help_message;
        root_->addUnlock(res);
        return res;
    }

    void merge(const OptionsEasy& other) { 
        for(auto& it : other.root_->unlocks()) {
            root_->addUnlock(it);
        }
        help_.merge(other.help_); 
    }

    void parse(std::vector<std::string> args) {
        Parser parser{root_};
    }

    std::vector<std::string> getCompletionVariants(std::vector<std::string> args) {
        Completer completer{root_};
        return completer.getCompletionVariants(args);
    }
    void check() {
        Checker checker;
        checker(root_);
    }    

    // ── Getters ──────────────────────────────────────────────
    HelpStringsStorage& help() { return help_; }
    const HelpStringsStorage& help() const { return help_; }

    ValueStorage& storage() { return storage_; }
    const ValueStorage& storage() const { return storage_; }

    std::shared_ptr<AbstractOption>& opt() { return root_; }
    const std::shared_ptr<AbstractOption>& opt() const { return root_; }

    // ── Setters ──────────────────────────────────────────────
    void setHelp(HelpStringsStorage help) { help_ = std::move(help); }
    void setStorage(ValueStorage storage) { storage_ = std::move(storage); }
    void setRoot(std::shared_ptr<AbstractOption> root) { root_ = std::move(root); }


   protected:
    HelpStringsStorage help_;
    ValueStorage storage_;
    std::shared_ptr<AbstractOption> root_;

    template <class OptionClass>
    std::shared_ptr<OptionClass> makeOptionWithName(const std::string& names) {
        auto [long_name, short_name] = splitToLongAndShortNames(names);
        std::shared_ptr<OptionClass> res;
        if (!long_name.has_value()) {
            throw std::logic_error("long option name is unspecified");
        }
        if (short_name.has_value()) {
            res = std::make_shared<OptionClass>(long_name.value(), short_name.value());
        } else {
            res = std::make_shared<OptionClass>(long_name.value());
        }
        return res;
    }
};

} /* namespace program_options_heavy */

#endif