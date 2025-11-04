/*!
hypercube --help
hypercube run --help
hypercube gather --help
hypercube run 
    --help 
    -d 1 -m 1 outputfile
    --bitsRepack 
        --srcLittleEndian
        --dstLittleEndian

helpCommand = command("help,h")
alt1 = opts.addAlternatives(helpCommand)
alt2 = opts.addAlternatives("run")
alt2.addCompatible("dim,d")
alt2.addCompatible("m")
repack = command("repack").
    addCompatibleChild("srcLittleEndian").
    addCompatibleChild("dstLittleEndian")
alt2.addCompatible(repack)



*/
#include <iostream>
#include <optional>
#include <map>
#include <memory>
#include <vector>
#include <variant>
#include <cassert>

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
        virtual void visit(std::shared_ptr<Alternatives>) = 0;
};

class AbstractOption : public std::enable_shared_from_this<AbstractOption> {
    /*struct Private {
        explicit Private() = default;
    };*/
    public:
        std::vector<std::shared_ptr<AbstractOption>> unlocks;
        std::shared_ptr<AbstractOption> addUnlock(std::shared_ptr<AbstractOption> opt) {
            unlocks.push_back(opt);
            return shared_from_this();
        }
        std::shared_ptr<AbstractOption> addAlternatives(std::shared_ptr<AbstractOption> alt1, std::shared_ptr<AbstractOption> alt2);
        std::shared_ptr<AbstractOption> addAlternatives(std::shared_ptr<AbstractOption> alt1, std::shared_ptr<AbstractOption> alt2, std::shared_ptr<AbstractOption> alt3);

        virtual void accept(AbstractOptionVisitor& visitor) {
            visitor.visit(shared_from_this());
        }
};

class AbstractNamedOption : public AbstractOption {
    public:
        std::optional<std::string> long_name;
        std::optional<std::string> short_name;

        AbstractNamedOption() {}
        AbstractNamedOption(const std::string& name) : long_name{name} {}
        AbstractNamedOption(const std::string& name, const std::string& sname) : long_name{name}, short_name{sname} {}
        void accept(AbstractOptionVisitor& visitor) override {
            visitor.visit(std::static_pointer_cast<AbstractNamedOption>(shared_from_this()));
        }
};

class AbstractPositionalOption : public AbstractOption {
    public:
        void accept(AbstractOptionVisitor& visitor) override {
            visitor.visit(std::static_pointer_cast<AbstractPositionalOption>(shared_from_this()));
        }
};

class Alternatives : public AbstractOption {
    public:
        std::vector<std::shared_ptr<AbstractOption>> alternatives;
        std::shared_ptr<AbstractOption> addAlternative(std::shared_ptr<AbstractOption> opt) {
            alternatives.push_back(opt);
            return shared_from_this();
        }
        void accept(AbstractOptionVisitor& visitor) override {
            visitor.visit(std::static_pointer_cast<Alternatives>(shared_from_this()));
        }
};

std::shared_ptr<AbstractOption> AbstractOption::addAlternatives(std::shared_ptr<AbstractOption> alt1, std::shared_ptr<AbstractOption> alt2) {
    auto opt = std::make_shared<Alternatives>();
    opt->addAlternative(alt1);
    opt->addAlternative(alt2);
    unlocks.push_back(opt);
    return shared_from_this();
}

std::shared_ptr<AbstractOption> AbstractOption::addAlternatives(std::shared_ptr<AbstractOption> alt1, std::shared_ptr<AbstractOption> alt2, std::shared_ptr<AbstractOption> alt3) {
    auto opt = std::make_shared<Alternatives>();
    opt->addAlternative(alt1);
    opt->addAlternative(alt2);
    opt->addAlternative(alt3);
    unlocks.push_back(opt);
    return shared_from_this();
}

struct Value {
    
};



class Matcher : public AbstractOptionVisitor {
    public:
        const std::string_view& str_;
        std::optional<size_t> match_index;
        std::vector<std::shared_ptr<AbstractOption>> unlocks;
        Matcher(const std::string_view& str) : str_{str} {};

        void visit(std::shared_ptr<AbstractOption> opt) override {
            match_index = std::nullopt;
            unlocks = opt->unlocks;
            assert(false);
        }
        void visit(std::shared_ptr<AbstractNamedOption> opt) override {
            assert(str_.size() > 0);
            match_index = std::nullopt;
            unlocks.clear();
            if(opt->long_name == str_ || opt->short_name == str_) {
                match_index = 0;
                unlocks = opt->unlocks;
            };
        }
        void visit(std::shared_ptr<AbstractPositionalOption>) override {
            match_index = std::nullopt;
            unlocks.clear();
            assert(false);
        }
        void visit(std::shared_ptr<Alternatives> opt) override {
            match_index = std::nullopt;
            unlocks.clear();
            for(size_t n = 0; n < opt->alternatives.size(); n++) {
                opt->alternatives[n]->accept(*this);
                if(match_index.has_value()) {
                    match_index = n;
                    unlocks = opt->alternatives[n]->unlocks;
                    return;
                }
            }
        }
};

template<class T>
std::ostream& operator<<(std::ostream& out, const std::optional<T>& v) {
    if(v.has_value()) {
        out<<*v;
    } else {
        out<<"(empty)";
    }
    return out;
}

class Printer : public AbstractOptionVisitor {
    public:
        std::string prefix;

        void visit(std::shared_ptr<AbstractOption> opt) override {
            assert(false);
        }
        void visit(std::shared_ptr<AbstractNamedOption> opt) override {
            std::cout<<prefix<<opt->short_name<<" : "<<opt->long_name<<std::endl;
            std::cout<<prefix<<"unlocks:"<<std::endl;
            auto old_prefix{prefix};
            prefix = std::string(" _ ") + prefix;
            for(auto it : opt->unlocks) {                
                it->accept(*this);
            }
            prefix = old_prefix;
        }
        void visit(std::shared_ptr<AbstractPositionalOption>) override {
            assert(false);
        }
        void visit(std::shared_ptr<Alternatives> opt) override {
            std::cout<<"Alternatives"<<std::endl;
            auto old_prefix{prefix};
            prefix = std::string("  ") + prefix;
            for(auto it : opt->alternatives) {
                it->accept(*this);
            }
            std::cout<<old_prefix<<"unlocks:"<<std::endl;
            for(auto it : opt->unlocks) {                
                it->accept(*this);
            }
            prefix = old_prefix;
        }
};

class Parser {
    public:
        std::map<std::shared_ptr<AbstractOption>, Value> values;
        std::vector<std::shared_ptr<AbstractOption>> possible_options_;

        Parser(std::shared_ptr<AbstractOption> possible_option) {
            possible_options_.push_back(possible_option);
        }
        
        bool parse(std::vector<std::string> tokens, size_t start_idx = 0) {
            if(start_idx == tokens.size()) {
                /// todo: check that all required options are used
                return true;
            }
            Matcher matcher(tokens[start_idx]);
            for(auto p : possible_options_) {
                p->accept(matcher);
                if(matcher.match_index.has_value()) {
                    if(values.contains(p)) {
                        assert(false); // duplicate parameter
                    }
                    values.insert(std::make_pair(p, Value{}));
                    possible_options_.insert(possible_options_.end(), matcher.unlocks.begin(), matcher.unlocks.end());
                    return parse(tokens, start_idx+1);
                }                
            }
            return false;
        }
};





int main(int argc, char* argv[]) {
    auto helpOption = std::make_shared<AbstractNamedOption>("--help", "-h");
    auto hypercubeOptions = std::make_shared<Alternatives>();
    hypercubeOptions->addAlternatives(
        helpOption,
        std::make_shared<AbstractNamedOption>("run")->
            addUnlock(std::make_shared<AbstractNamedOption>("--dim", "-d"))->
            addUnlock(std::make_shared<AbstractNamedOption>("--mIntervalsPerDim", "-m")),
        std::make_shared<AbstractNamedOption>("gather")
    );

    Printer printer;
    hypercubeOptions->accept(printer);

    Parser parser(hypercubeOptions);
    std::cout<<parser.parse({"run", "--dim"})<<std::endl;

    return 0;
}