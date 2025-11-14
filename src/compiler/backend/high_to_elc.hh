#ifndef HIGH_TO_ELC_HH_
#define HIGH_TO_ELC_HH_

// TODO: VisitValue does not need to be in sema.hh
#include "../frontend/sema.hh"
#include "lambda.hh"

struct TreeToELCVisitor : ITreeVisitor
{
    TreeToELCVisitor()
        : v_elc()
    {}

    virtual void visit(TreeSeqNode *node);
    virtual void visit(TreeParamsNode *node);
    virtual void visit(TreeListNode *node);
    virtual void visit(TreeBindingNode *node);
    virtual void visit(TreeBinopNode *node);
    virtual void visit(TreeApplyNode *node);
    virtual void visit(TreeMatchNode *node);
    virtual void visit(TreeMatchArmNode *node);
    virtual void visit(TreeIdentNode *node);
    virtual void visit(TreeIntegerNode *node);
    virtual void visit(TreeStringNode *node);

    VisitValue<LCNodePtr> v_elc;

    // We would like to return (v = B) for a top-level `let` and (let v = B in E) otherwise
    VisitValue<bool> v_is_toplevel;
};

#endif // HIGH_TO_ELC_HH_
