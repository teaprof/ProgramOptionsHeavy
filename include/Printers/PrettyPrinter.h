#ifndef __PRETTY_PRINTER_H__
#define __PRETTY_PRINTER_H__

#include <Printers/Document.h>

#include <iostream>
#include <locale>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace program_options_heavy
{

namespace printers
{

class PrettyPrinter : public DocumentVisitor
{
  public:
    void visit(const AbstractItem &item) override
    {
        assert(false);
    }
    void visit(const Paragraph &item) override
    {
        printText(level, item.text);
    }
    void visit(const UnorderedList &lst) override
    {
        for (const auto &item : lst.items)
        {
            printText(level, std::string("*") + item);
        }
    }
    void visit(const Section &item) override
    {
        printTitle(level, item.title);
        level++;
        for (const auto &item : item.items)
        {
            item->accept(*this);
        }
        level--;
    }

    void print(const std::shared_ptr<AbstractItem> item)
    {
        item->accept(*this);
    }

  private:
    size_t level{0};

    static void printTitle(size_t level, const std::string &str)
    {
        if (str.empty())
        {
            return;
        }
        std::cout << "\n" << print(titleprefix(level), toUpper(str), titlesuffix(level)) << std::endl;
    }
    static void printText(size_t level, const std::string &str)
    {
        if (str.empty())
        {
            return;
        }
        std::cout << print(textprefix(level), str, textsuffix(level)) << std::endl;
    }
    static std::string print(const std::string &lineprefix, const std::string_view &view, const std::string &linesuffix)
    {
        if (view.empty())
        {
            return "";
        }
        std::stringstream res;
        res << lineprefix;
        for (auto ch : view)
        {
            if (ch == '\n')
            {
                res << linesuffix << "\n";
                res << lineprefix;
            }
            else
            {
                res << ch;
            }
        }
        res << linesuffix;
        return res.str();
    }
    static std::string titleprefix(size_t level)
    {
        if (level <= 1)
        {
            return repeat(level, "  ") + bold();
        };
        return repeat(level, "  ");
    }
    static std::string titlesuffix(size_t level)
    {
        return reset();
    }
    static std::string textprefix(size_t level)
    {
        return repeat(level + 1, "  ");
    }
    static std::string textsuffix(size_t level)
    {
        return "";
    }
    static std::string repeat(size_t level, std::string pattern = " ")
    {
        std::string res;
        for (size_t n = 0; n < level; n++)
            res += pattern;
        return res;
    }
    static std::string toUpper(const std::string_view &str)
    {
        std::string res;
        for (auto ch : str)
        {
            res += std::toupper(ch);
        }
        return res;
    }
    // For esc-codes, see
    // https://man7.org/linux/man-pages/man4/console_codes.4.html
    static std::string underline()
    {
        return "\033[4m";
    }
    static std::string bold()
    {
        return "\033[1m";
    }
    static std::string red()
    {
        return "\033[31m";
    }
    static std::string reset()
    {
        return "\033[0m";
    }
};

} /* namespace printers */

} /* namespace program_options_heavy */

#endif