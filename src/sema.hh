#ifndef SEMA_HH_
#define SEMA_HH_

#include "common.hh"
#include "tree.hh"
#include "symtab.hh"

/*
  Answers a question about the immediate accept() call.
  Resets on being read.
 */
template <class T>
struct VisitValue
{
    VisitValue()
        : value(), valid(false)
    {}

    void write(const T& v)
    {
        value = v;
        valid = true;
    }

    std::optional<T> read()
    {
        if (!valid) return {};

        // Values can only be read once.
        valid = false;

        return value;
    }

    T read_asserted()
    {
        auto opt = read();
        assert(opt.has_value());
        return *opt;
    }

    T read_or(const T& _default)
    {
        auto opt = read();
        if (opt.has_value())
            return *opt;
        return _default;
    }

    bool is_valid()
    {
        return valid;
    }

    void erase()
    {
        valid = false;
    }
    
    T value;
    bool valid;
};

struct TreeSemaVisitor : ITreeVisitor
{
    TreeSemaVisitor(SymbolTable& table)
        : table(std::move(table)), valid(true), v_type(), v_insert(), v_arity()
    {}
    
    virtual void visit(TreeSeqNode *node);
    virtual void visit(TreeParamsNode *node);
    virtual void visit(TreeListNode *node);
    virtual void visit(TreeBindingNode *node);
    virtual void visit(TreeBinopNode *node);
    virtual void visit(TreeApplyNode *node);
    virtual void visit(TreeIdentNode *node);
    virtual void visit(TreeIntegerNode *node);
    virtual void visit(TreeStringNode *node);

    SymbolTable table;
    bool valid;

    VisitValue<Type *> v_type;
    VisitValue<bool>   v_insert;
    VisitValue<size_t> v_arity;
};

#endif // SEMA_HH_
