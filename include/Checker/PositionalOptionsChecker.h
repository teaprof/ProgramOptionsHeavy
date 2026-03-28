#ifndef __CHECKER_POSITIONAL_OPTIONS_CHECKER_H__
#define __CHECKER_POSITIONAL_OPTIONS_CHECKER_H__

#include <Backend/Option.h>

class PositionalOptionsChecker {
    public:
        static void checkPositionalOptionCouldBeResolved(std::vector<std::shared_ptr<AbstractOption>> opts) {
            std::vector<std::shared_ptr<AbstractPositionalOption>> positional;
            for(const auto& it : opts)
                if(auto p = std::dynamic_pointer_cast<AbstractPositionalOption>(it)) {
                    positional.push_back(p);
                }
            /// TODO check that SLAE could be solved
            /*if(state_.has_positional_option_with_multiple_occurrence) {
                throw MultipleOccurenceOnlyForLastPosopt(opt);
            }
            if(opt->maxOccurrence() != 1) {
                state_.has_positional_option_with_multiple_occurrence = true;
            }
            visit(std::static_pointer_cast<AbstractOption>(opt));*/
        }
};

#endif // __CHECKER_POSITIONAL_OPTIONS_CHECKER_H__