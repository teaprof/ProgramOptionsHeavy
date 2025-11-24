#ifndef __OPTIONS_GROUP_H__
#define __OPTIONS_GROUP_H__

#include <Backend/Facade.h>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>

#include <cstdint>

namespace program_options_heavy
{

class OptionsGroup : public OptionsFacade
{
  public:
    OptionsGroup(std::string group_name)
    {
        setGroupName(tolower(group_name));
        options = std::make_shared<OptionsGroup2>();
    }

    template <class ... Args>
    auto addPartial(Args... args)
    {
        auto opt = makeOption(args...);
        options->addUnlock(opt);

        return opt;
    }
    template <class T>
    auto addPositional(std::string name, int count, std::reference_wrapper<T> external_storage)
    {
        auto opt = makePositionalOption(name, count, external_storage);
        options->addUnlock(opt);

        return opt;
    }
    virtual void validate() // TODO: remove it?
    {
        // nothing to do
        // redefine this function in the derived class
    }
    virtual void update(const boost::program_options::variables_map &vm) // TODO: remove it?
    {
        // nothing to do
        // redefine this function in the derived class
    }
    void setGroupName(std::string str)
    {
        help_.setGroupName(options, str);
        group_name_ = std::move(str);
    }
    const std::string &groupName() const
    {
        return group_name_;
    }
    void setGroupDescription(std::string str) {
        help_<<options<<str;
    }
    std::string description() {
        if(help_.help_strings_.count(options) > 0)
            return help_.help_strings_[options];
        return "";
    }
    std::shared_ptr<OptionsGroup2> options;
  private:
    std::string group_name_;
};

} /* namespace program_options_heavy */

#endif //__OPTIONS_GROUP_H__