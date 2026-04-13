#ifndef CHECKER_CHECKER_H
#define CHECKER_CHECKER_H

#include "Combinator.h"
#include "Exceptions.h"
#include "PairwiseCheckers.h"
#include "PositionalOptionsChecker.h"
#include "UnoCheckers.h"
#include <sstream>
#include <stdexcept>

class Checker
{
    // TODO checker should check
    // - if the default value is within [min, max]
    // - if the default value satisfies regex
    // - if the default value is contained in Unlocks
    // - the same for implicit value
    // - hidden names of positional options differes from other names
  public:
    Checker()
    {
    }
    void operator()(std::shared_ptr<AbstractOption> opt)
    {
        Combinator combinator(opt);
        Unochecker unocheker;
        PairwiseChecker pairwise_checker;
        for (auto &branch : combinator.branches)
        {
            for (auto &opt : branch)
            {
                opt->accept(unocheker);
            }
            for (size_t i = 0; i < branch.size(); i++)
            {
                for (size_t j = i + 1; j < branch.size(); j++)
                {
                    pairwise_checker.check(branch[i], branch[j]);
                }
            }
        }

        /// TODO: check positional option
    }

  private:
};
#endif