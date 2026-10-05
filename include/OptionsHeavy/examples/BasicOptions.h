#ifndef PARSERS_BASICOPTIONS_H
#define PARSERS_BASICOPTIONS_H

#include <OptionsHeavy/basic/HeavyOption.h>
#include <Printers/PrettyPrinter.h>

#include <boost/optional.hpp>
#include <iostream>
#include <thread>
#include <vector>

namespace program_options_heavy {

class CommonOptions : public HeavyOptionsGroup {
   public:
    enum CommonOptionsFlags {
        HelpEnabled = 1 << 0,
        AutoCompletionEnabled = 1 << 1,
    };
    CommonOptions(const CommonOptionsFlags& flags) : HeavyOptionsGroup("Common options") {
        if(flags & HelpEnabled) {
            addPartial("help", std::ref(need_help_), "produce this help")->
                valueSemantics().setDefaultValue(false).setImplicitValue(true);
        }
        if(flags & AutoCompletionEnabled) {
            // TODO: exclude this option from autocompletion
            auto option = addPartial("autocomplete", std::ref(auto_completion_mode_), "Special option that is used to actiave autocomplete feature");
            option->valueSemantics().setDefaultValue(false).setImplicitValue(true);
            auto_completion_args = std::make_shared<PositionalOptionWithValue<std::string>>();
            auto_completion_args->setNValues(AbstractOptionWithValue::NValuesRole::INFINITE);            
            option->addUnlock(auto_completion_args);
        }        
    }
    void update(const boost::program_options::variables_map& vm) override {
        // need_help = vm.count("help") > 0;
    }
    bool needHelp() const { return need_help_; }

    // can be used in some parsers if argc == 1 (only exe name without arguments)
    void setNeedHelp(bool value) {        
        need_help_ = value;
    }
    bool autoCompletionMode() {
        return auto_completion_mode_;
    }
    std::vector<std::string> getAutoCompletionArgs(const KeyValueStorage& storage) {        
        if(storage.contains(auto_completion_args)) {
            const auto& value_storage = storage[auto_completion_args];
            assert(value_storage.occurrenceCount() == 1);
            return value_storage.rawValuesVec(0);
        }
        return {};
    }
   private:
    bool need_help_{false};
    bool auto_completion_mode_{false};
    std::shared_ptr<PositionalOptionWithValue<std::string>> auto_completion_args;
};


class MultithreadOptions : public HeavyOptionsGroup {
   public:
    MultithreadOptions() : HeavyOptionsGroup("multithreading options") {
        namespace po = boost::program_options;
        concurency_ = std::thread::hardware_concurrency();
        std::stringstream str;
        str << "number of threads, if not set then "
               "std::threads::hardware_concurrency will be used ["
            << concurency_ << " on this machine]";
        // addPartialVisible("nthreads,t", po::value(&nthreads_),
        // str.str().c_str());
        addPartial("nthreads,t", std::ref(nthreads_), str.str());
    }
    size_t nThreads() {
        if (nthreads_) {
            return *nthreads_;
        }
        return concurency_;
    }

   private:
    size_t concurency_;
    std::optional<size_t> nthreads_;
};


} /* namespace program_options_heavy */

#endif