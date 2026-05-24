#ifndef __DYNAMIC_OPTIONS_CONTAINER_H__
#define __DYNAMIC_OPTIONS_CONTAINER_H__
#include <Backend/Parser.h>
#include <OptionsHeavy/basic/HeavyOption.h>

namespace program_options_heavy {

class HeavyParser : public Parser {
    public:
        //void onNewOccurrenceFinished(std::shared_ptr<>)
        void onDefaultValueApplied(std::shared_ptr<AbstractOptionWithValue> option) override {
            for(auto& opt : options_) {
                if(opt->opt() ==  option) {
                    opt->onDefaultValueApplied();
                    return;
                }
            }
            throw std::logic_error("This line should not be reached");
        }
        void onImplicitValueApplied(std::shared_ptr<AbstractOptionWithValue> opt) override {

        }
        void onValueApplied(std::shared_ptr<AbstractOptionWithValue> opt) override {

        }
    private:
        std::vector<std::shared_ptr<HeavyOptionBase>> options_;
}

#endif