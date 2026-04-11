#ifndef PARSERS_BASICOPTIONS_H
#define PARSERS_BASICOPTIONS_H

#include <Printers/PrettyPrinter.h>
#include <Parsers/OptionsGroup.h>
#include <boost/optional.hpp>
#include <iostream>
#include <thread>
#include <vector>

namespace program_options_heavy
{

class HelpOptions : public OptionsGroup
{
  public:
    HelpOptions() : OptionsGroup("Help options")
    {
        namespace po = boost::program_options;
        //addPartialVisible("help", po::bool_switch(&need_help), "produce this help");
        addPartial("help", std::ref(need_help_), "produce this help");
    }
    void update(const boost::program_options::variables_map &vm) override
    {
        // need_help = vm.count("help") > 0;
    }
    bool needHelp() const
    {
        return need_help_;
    }

  private:
    bool need_help_;
};

class MultithreadOptions : public OptionsGroup
{
  public:
    MultithreadOptions() : OptionsGroup("multithreading options")
    {
        namespace po = boost::program_options;
        concurency_ = std::thread::hardware_concurrency();
        std::stringstream str;
        str << "number of threads, if not set then "
               "std::threads::hardware_concurrency will be used ["
            << concurency_ << " on this machine]";
        //addPartialVisible("nthreads,t", po::value(&nthreads_), str.str().c_str());
        addPartial("nthreads,t", std::ref(nthreads_), str.str());
    }
    size_t nThreads()
    {
        if (nthreads_)
        {
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