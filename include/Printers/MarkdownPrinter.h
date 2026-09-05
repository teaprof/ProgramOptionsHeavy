#ifndef PRINTERS_PRINTER_H
#define PRINTERS_PRINTER_H

#include <Printers/Document.h>

#include <iostream>
#include <cassert>
#include <iomanip>
//#include <locale>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace program_options_heavy {

namespace printers {

class MarkdownPrinter : public DocumentVisitor {
   public:   
    void visit(const AbstractItem& item) override { assert(false); }
    void visit(const Paragraph& item) override { printText(item.text); }
    void visit(const UnorderedList& lst) override {
        for (const auto& item : lst.items) {
            printText(std::string("* ") + item);
        }
    }
    void visit(const Section& item) override {
        level_++;
        printTitle(level_, item.title);
        for (const auto& item : item.items) {
            item->accept(*this);
        }
        level_--;
    }

    void visit(const Table& table) override {
        size_t ncols = table.ncols();

        for(size_t row = 0; row < table.nrows(); row++) {                        
            if(row == 1) {
                std::ostringstream oss;
                oss << "|";
                for(size_t col = 0; col < ncols; col++) {
                    oss << "---|";
                }
                printText(oss.str());                
            }

            std::ostringstream oss;
            oss << "|";
            for(size_t col = 0; col < ncols; col++) {
                oss << table.data(row, col);
                oss << " | ";
            }
            printText(oss.str());
        }
    }

    void print(const std::shared_ptr<AbstractItem> item) { item->accept(*this); }

   private:
    size_t level_{0};

    static void printTitle(size_t level, const std::string& str) {
        std::cout << "\n" << titleprefix(level) << str <<"\n";
    }
    static void printText(const std::string& str) {
        std::cout << str << std::endl;
    }
    static std::string titleprefix(size_t level) {
        return repeat(level, "#") + " ";
    }
    static std::string repeat(size_t level, std::string pattern = " ") {
        std::string res;
        for (size_t n = 0; n < level; n++) {
            res += pattern;
        }
        return res;
    }
};

} /* namespace printers */

} /* namespace program_options_heavy */

#endif