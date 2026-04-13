#ifndef CHECKER_COMBINATOR_H
#define CHECKER_COMBINATOR_H
#include <Backend/Option.h>
#include <memory>
#include <vector>
class Combinator2 : public AbstractOptionVisitor
{
  public:
    size_t level{0}; // this counter detects infinite recursion
    struct Node
    {
        std::shared_ptr<AbstractOption> option;
        std::vector<std::shared_ptr<Node>> children;
    };
    std::shared_ptr<Node> front{nullptr};
    std::vector<std::shared_ptr<Node>> ends;

    void pushBack(std::shared_ptr<AbstractOption> opt)
    {
        if (++level > 1000)
        {
            throw RecursionDetected();
        }
        auto node = std::make_shared<Node>();
        node->option = opt;
        if (front == nullptr)
        {
            front = node;
            assert(ends.empty());
            ends.push_back(front);
            return;
        }
        for (auto it : ends)
        {
            assert(it->children.empty());
            it->children.push_back(node);
        }
        ends.clear();
        ends.push_back(node);
    }
    void pushBack(std::vector<Combinator2> combinators)
    {
        assert(!combinators.empty());
        assert(!ends.empty());
        for (auto &it : ends)
        {
            assert(it->children.empty());
            for (auto &c : combinators)
            {
                it->children.push_back(c.front);
            }
        }
        ends.clear();
        for (auto &c : combinators)
        {
            ends.insert(ends.end(), c.ends.begin(), c.ends.end());
        }
    }

    virtual ~Combinator2() {};
    virtual void visit(std::shared_ptr<AbstractOption> opt)
    {
        pushBack(opt);
        for (const auto &it : opt->unlocks())
        {
            it->accept(*this);
        }
    }
    virtual void visit(std::shared_ptr<AbstractPositionalOption> opt)
    {
        visit(std::dynamic_pointer_cast<AbstractOption>(opt));
    }
    virtual void visit(std::shared_ptr<NamedOption> opt)
    {
        visit(std::dynamic_pointer_cast<AbstractOption>(opt));
    }
    virtual void visit(std::shared_ptr<LiteralString> opt)
    {
        visit(std::dynamic_pointer_cast<AbstractOption>(opt));
    }
    virtual void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt)
    {
        visit(std::dynamic_pointer_cast<AbstractOption>(opt));
    }
    virtual void visit(std::shared_ptr<AbstractPositionalOptionWithValue> opt)
    {
        visit(std::dynamic_pointer_cast<AbstractOption>(opt));
    }
    virtual void visit(std::shared_ptr<OptionsGroup2> opt)
    {
        visit(std::dynamic_pointer_cast<AbstractOption>(opt));
    }
    virtual void visit(std::shared_ptr<OneOf> oneof)
    {
        std::vector<Combinator2> combinators;
        assert(!oneof->alternatives.empty()); // todo: throw Alternatives is empty
        visit(std::dynamic_pointer_cast<AbstractOption>(oneof));
        for (auto it : oneof->alternatives)
        {
            Combinator2 c;
            it->accept(c);
            combinators.push_back(std::move(c));
        }
        pushBack(combinators);
    }
};

class Combinator
{
  public:
    std::vector<std::vector<std::shared_ptr<AbstractOption>>> branches;
    std::vector<std::shared_ptr<AbstractOption>> current_branch;
    Combinator(std::shared_ptr<AbstractOption> option)
    {
        Combinator2 combinator;
        option->accept(combinator);
        walk(combinator.front);
    }
    void walk(std::shared_ptr<Combinator2::Node> node)
    {
        current_branch.push_back(node->option);
        if (node->children.empty())
        {
            branches.push_back(current_branch);
        }
        else
        {
            for (auto &child : node->children)
            {
                walk(child);
            }
        }
        current_branch.pop_back();
    }
    struct Iterator
    {
        size_t idx;
        Combinator &parent;
        std::vector<std::shared_ptr<AbstractOption>> operator*()
        {
            return parent.branches[idx];
        }
        void operator++()
        {
            idx++;
        }
        bool operator!=(const Iterator &other) const
        {
            return idx != other.idx;
        }
    };
    Iterator begin()
    {
        return Iterator{0, *this};
    }
    Iterator end()
    {
        return Iterator{branches.size(), *this};
    }
};
#endif // CHECKER_COMBINATOR_H