#ifndef __OPTIONS_GROUP_H__
#define __OPTIONS_GROUP_H__

#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>

#include <cstdint>

namespace program_options_heavy
{

class OptionsGroup
{
  public:
    static constexpr uint16_t text_width = 140;

    OptionsGroup(std::string group_name) : visible(text_width)
    {
        std::string lower;
        for (auto ch : group_name)
        {
            lower += std::tolower(ch);
        }
        setGroupName(lower);
    }

    template <class... Args> auto addPartialVisible(Args... args)
    {
        // this option will be used in parsing and will be printed in the help
        // message
        auto option = boost::make_shared<boost::program_options::option_description>(args...);
        partial.add(option);
        visible.add(option);
        return option;
    }
    void addPositionalHidden(std::string name, int count, const boost::program_options::value_semantic *s)
    {
        // this option will be used in parsing, but will not be printed in the help
        // message
        positional.add(name.c_str(), count);
        partial.add_options()(name.c_str(), s);
    }
    auto addPositionalVisible(std::string name, int count, const boost::program_options::value_semantic *s,
                              std::string description)
    {
        // this option will be used in parsing and will be printed in the help
        // message
        auto option =
            boost::make_shared<boost::program_options::option_description>(name.c_str(), s, description.c_str());
        positional.add(name.c_str(), count);
        partial.add(option);
        visible.add(option);
        return option;
    }

    virtual void validate()
    {
        // nothing to do
        // redefine this function in the derived class
    }
    virtual void update(const boost::program_options::variables_map &vm)
    {
        // nothing to do
        // redefine this function in the derived class
    }

    // These vars are used only for parsing arguments
    boost::program_options::options_description partial;
    boost::program_options::positional_options_description positional;

    // These vars are used only for printing help message
    boost::program_options::options_description visible;
    boost::program_options::options_description hidden;

    /*std::stringstream detailedList() const
    {
        std::stringstream res;
        res << visible;
        return res;
    }*/

    void setGroupName(std::string str)
    {
        group_name_ = std::move(str);
    }
    const std::string &groupName() const
    {
        return group_name_;
    }
    std::stringstream description;

  private:
    std::string group_name_;
    // bool not_specified_{true}; // true if none of the positional or partial
    // options are specified
};

} /* namespace program_options_heavy */

#endif //__OPTIONS_GROUP_H__