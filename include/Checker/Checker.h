#ifndef __BACKEND_CHECKER_H__
#define __BACKEND_CHECKER_H__

#include "Exceptions.h"
#include "Combinator.h"
#include "UnoCheckers.h"
#include "PairwiseCheckers.h"
#include "PositionalOptionsChecker.h"
#include <sstream>
#include <stdexcept>

class Checker {
// TODO checker should check 
// - if the default value is within [min, max]
// - if the default value satisfies regex
// - if the default value is contained in Unlocks
// - the same for implicit value
// - hidden names of positional options differes from other names
public:
    Checker() {}
    void operator()(std::shared_ptr<AbstractOption> opt) {
        Combinator combinator(opt);
        Unochecker unocheker;
        PairwiseChecker pairwiseChecker;
        for(auto& branch : combinator.branches) {
            for(auto& opt : branch) {
                opt->accept(unocheker);
            }
            for(size_t i = 0; i < branch.size(); i++) {
                for(size_t j = i + 1; j < branch.size(); j++) {
                    pairwiseChecker.check(branch[i], branch[j]);
                }
            }
        }

        /// TODO: check positional option
    }
private:
};
#endif