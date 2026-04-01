#ifndef __CHECKER_POSITIONAL_OPTIONS_CHECKER_H__
#define __CHECKER_POSITIONAL_OPTIONS_CHECKER_H__

#include <Backend/Option.h>

class PositionalOptionsChecker {
    public:
        static void checkPositionalOptionCouldBeResolved(std::vector<std::shared_ptr<AbstractPositionalOption>> opts) {
            for(size_t n = 0; n + 1 < opts.size(); n++) {
                if(opts[n]->maxOccurrence() > 1) {
                    // TODO: Since the correct way to distribute arguments among positional option involves SLAE solving,
                    // these check should be controlled by flag: SLAU_SOLVE or LAST_POSITIONAL_OPTION_EAT_ALL
                    throw MultipleOccurenceOnlyForLastPosopt(opts[n]);
                }                
            }
        }
};

#endif // __CHECKER_POSITIONAL_OPTIONS_CHECKER_H__
