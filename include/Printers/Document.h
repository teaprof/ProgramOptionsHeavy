#ifndef __ABSTRACT_TEXT_SECTION__
#define __ABSTRACT_TEXT_SECTION__

#include <memory>
#include <sstream>
#include <vector>

namespace program_options_heavy
{

namespace printers
{

class AbstractItem;
class Paragraph;
class UnorderedList;
class Section;

class DocumentVisitor
{
  public:
    virtual void visit(const AbstractItem &item) = 0;
    virtual void visit(const Paragraph &item) = 0;
    virtual void visit(const UnorderedList &item) = 0;
    virtual void visit(const Section &item) = 0;
};

class AbstractItem
{
  public:
    virtual void accept(DocumentVisitor &visitor) = 0;
};

class Paragraph : public AbstractItem
{
  public:
    Paragraph()
    {
    }
    Paragraph(const std::string &str) : text{str}
    {
    }
    std::string text;
    void accept(DocumentVisitor &visitor) override
    {
        visitor.visit(*this);
    }
};

class UnorderedList : public AbstractItem
{
  public:
    std::vector<std::string> items;
    void accept(DocumentVisitor &visitor) override
    {
        visitor.visit(*this);
    }
};

class Section : public AbstractItem
{
  public:
    std::string title;
    std::vector<std::shared_ptr<AbstractItem>> items;
    void add_paragraph(const std::string &str)
    {
        items.push_back(std::make_shared<Paragraph>(str));
    }
    void accept(DocumentVisitor &visitor) override
    {
        visitor.visit(*this);
    }
};

} /* namespace printers */

} /* namespace program_options_heavy */

#endif // __ABSTRACT_TEXT_SECTION__