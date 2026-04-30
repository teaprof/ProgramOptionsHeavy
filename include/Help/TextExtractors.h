#ifndef HELP_TEXTEXTRACTORS_H
#define HELP_TEXTEXTRACTORS_H

#include <Help/HelpStrings.h>
#include <Help/TextualDescriptions.h>
#include <OptionsHeavy/Parser.h>

#include <map>
#include <optional>
#include <sstream>

class OptionTextExtractor : public AbstractOptionVisitor {
   public:
    OptionTextExtractor() {}
    OptionTextExtractor(const HelpStrings& help) : help_(help) {}
    void visit(std::shared_ptr<AbstractOption> opt) {
        // assert(false);
    }
    void visit(std::shared_ptr<AbstractPositionalOption> opt) {
        descr = ParameterDescription{{"some positional option"}, helpString(opt)};
    }
    void visit(std::shared_ptr<NamedOption> opt) { descr = ParameterDescription{displayName(opt), helpString(opt)}; }
    void visit(std::shared_ptr<LiteralString> opt) { descr = ParameterDescription{opt->str() + "(literal)", helpString(opt)}; }
    void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) {
        descr = ParameterDescription{displayName(opt), helpString(opt)};
    }
    void visit(std::shared_ptr<AbstractPositionalOptionWithValue> opt) {
        descr = ParameterDescription{{"some positional option = value"}, helpString(opt)};  // TODO implement this
    }
    void visit(std::shared_ptr<OptionsGroup2> opt) { descr = ParameterDescription{groupName(opt), helpString(opt)}; }
    void visit(std::shared_ptr<OneOfPositional> opt) {
        descr = ParameterDescription{{"one of"}, helpString(opt)};  // TODO implement this
    }
    void visit(std::shared_ptr<OneOfNamed> opt) {
        descr = ParameterDescription{{"one of"}, helpString(opt)};  // TODO implement this
    }
    ParameterDescription descr;
    std::string str() const { return descr.keys + "\n" + descr.description; }

   private:
    std::optional<std::reference_wrapper<const HelpStrings>> help_;
    std::string displayName(std::shared_ptr<NamedOption> opt) {
        if (help_) {
            if (help_->get().key_strings.contains(opt)) {
                return help_->get().key_strings.at(opt);
            }
        }
        std::string res;
        std::string delimiter;
        if (opt->longName()) {
            res = *opt->longName();
            delimiter = ", ";
        };
        if (opt->shortName()) {
            res += delimiter + *opt->shortName();
        };
        return res;
    }
    std::string helpString(std::shared_ptr<AbstractOption> opt) {
        if (help_) {
            if (help_->get().help_strings.contains(opt)) {
                return help_->get().help_strings.at(opt);
            }
        }
        return "";
    }
    std::string groupName(std::shared_ptr<OptionsGroup2> opt) {
        if (help_) {
            if (help_->get().group_descriptions.contains(opt)) {
                return help_->get().group_descriptions.at(opt).name;
            }
        }
        return "[group]";  // TODO group1 group2 ....
    }
};

class Extractor {
   public:
    EntireDescription extract(HelpStrings& help, std::shared_ptr<AbstractOption> opt) {
        EntireDescription res;
        res.program_description = extractProgramDescription(help, opt);
        res.run_variants = extractRunVariants(help, opt);
        res.group_descriptions = extractGroupDescriptions(help, opt);
        res.parameter_descriptions = extractParameterDescriptions(help, opt);
        return res;
    }
    virtual ProgramDescription extractProgramDescription(const HelpStrings& help, std::shared_ptr<AbstractOption> opt) {
        return help.program_description;
    }
    virtual RunVariantsDescription extractRunVariants(const HelpStrings& help, std::shared_ptr<AbstractOption> opt) = 0;
    virtual std::vector<GroupDescription> extractGroupDescriptions(const HelpStrings& help, std::shared_ptr<AbstractOption> opt) {
        std::vector<GroupDescription> res;
        for (const auto& it : help.group_descriptions) {
            res.push_back(it.second);
        }
        return res;
    }
    virtual std::vector<ParameterDescription> extractParameterDescriptions(const HelpStrings& help,
                                                                           std::shared_ptr<AbstractOption> opt) {
        return {};
    }
};

class SimpleExtractor : public Extractor {
   public:
    SimpleExtractor(const program_options_heavy::Parser& parser) : parser_{parser} {}
    RunVariantsDescription extractRunVariants(const HelpStrings& help, std::shared_ptr<AbstractOption> opt) override {
        std::string exename = "123.exe";                            // TODO
        std::string brief = "run variant brief description";        // TODO
        std::string detailed = "run variant detailed description";  // TODO
        std::vector<std::string> args;
        for (const auto& grp : parser_.groups()) {
            args.push_back(help.getGroupName(grp->options));
        }
        RunVariantDescription descr{exename, brief, detailed, args};
        RunVariantsDescription res;
        res.variants.push_back(descr);
        return res;
    }

   private:
    program_options_heavy::Parser parser_;
};

#endif