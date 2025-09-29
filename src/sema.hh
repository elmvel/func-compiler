#ifndef SEMA_HH_
#define SEMA_HH_

#include "common.hh"
#include "tree.hh"
#include "symtab.hh"

struct TreeSemaVisitor : ITreeVisitor
{
    TreeSemaVisitor(SymbolTable& table)
        : table(std::move(table))
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
    Type v_type;
    bool v_insert;
};

#endif // SEMA_HH_
