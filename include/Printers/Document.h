#ifndef PRINTERS_DOCUMENT_H
#define PRINTERS_DOCUMENT_H

#include <cassert>
#include <memory>
#include <sstream>
#include <vector>

namespace program_options_heavy {

namespace printers {

class AbstractItem;
class Paragraph;
class UnorderedList;
class Table;
class Section;

class DocumentVisitor {
   public:
    virtual void visit(const AbstractItem& item) = 0;
    virtual void visit(const Paragraph& item) = 0;
    virtual void visit(const UnorderedList& item) = 0;
    virtual void visit(const Section& item) = 0;
    virtual void visit(const Table& item) = 0;
};

/*class Document {
    public:
        std::vector<std::shared_ptr<AbstractItem>> items;
}*/

class AbstractItem {
   public:
    virtual void accept(DocumentVisitor& visitor) = 0;
};

class Paragraph : public AbstractItem {
   public:
    Paragraph() {}
    Paragraph(const std::string& str) : text{str} {}
    std::string text;
    void accept(DocumentVisitor& visitor) override { visitor.visit(*this); }
};

class UnorderedList : public AbstractItem {
   public:
    std::vector<std::string> items;
    void accept(DocumentVisitor& visitor) override { visitor.visit(*this); }
};

class Table : public AbstractItem {
    public:
    std::vector<int> align_; // -1 for left, +1 for right
    std::vector<std::vector<std::string>> rows;
    void accept(DocumentVisitor& visitor) override { visitor.visit(*this); }
    void addRow() {
        rows.push_back(std::vector<std::string>());
    }
    void addCellToRow(const std::string& text) {
        rows.back().push_back(text);
    }

    std::string data(size_t row, size_t col) const {
        assert(row < rows.size());
        if(col >= rows[row].size()) {
            // if row size is exceeded return empty string
            return "";
        }
        return rows[row][col];        
    }

    size_t maxColumnWidth(size_t col) const {
        size_t res = 0;
        for(const auto& row : rows) {
            if(col < row.size()) {
                res = std::max(res, row[col].size());
            }
        }
        return res;
    }

    size_t ncols() const {
        size_t res{0};
        for(const auto& row : rows) {
            res = std::max(res, row.size());
        }
        return res;
    }
    size_t nrows() const {
        return rows.size();
    }
    int align(size_t col) const {
        if(col >= align_.size()) {
            return -1;
        }
        return align_[col];
    }
};

class Section : public AbstractItem {
   public:
    std::string title;
    std::vector<std::shared_ptr<AbstractItem>> items;
    void addParagraph(const std::string& str) { items.push_back(std::make_shared<Paragraph>(str)); }
    void addTable(std::shared_ptr<Table> table) { items.push_back(table); }
    void addUnorderedList(std::shared_ptr<UnorderedList> ul) { items.push_back(ul); }
    void accept(DocumentVisitor& visitor) override { visitor.visit(*this); }
};


} /* namespace printers */

} /* namespace program_options_heavy */

#endif  // PRINTERS_DOCUMENT_H