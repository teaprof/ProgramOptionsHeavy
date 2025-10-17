#ifndef BASIC_OPTIONS_H
#define BASIC_OPTIONS_H

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
        addPartialVisible("help", po::bool_switch(&need_help), "produce this help");
    }
    void update(const boost::program_options::variables_map &vm) override
    {
        // need_help = vm.count("help") > 0;
    }
    bool needHelp() const
    {
        return need_help;
    }

  private:
    bool need_help;
};

class MultithreadOptions : public OptionsGroup
{
  public:
    MultithreadOptions() : OptionsGroup("multithreading options")
    {
        namespace po = boost::program_options;
        concurency = std::thread::hardware_concurrency();
        std::stringstream str;
        str << "number of threads, if not set then "
               "std::threads::hardware_concurrency will be used ["
            << concurency << " on this machine]";
        addPartialVisible("nthreads,t", po::value(&nthreads_), str.str().c_str());
    }
    size_t nThreads()
    {
        if (nthreads_)
        {
            return *nthreads_;
        }
        return concurency;
    }

  private:
    size_t concurency;
    boost::optional<size_t> nthreads_;
};

} /* namespace program_options_heavy */

#endif