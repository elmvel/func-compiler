#ifndef HIGH_TO_ELC_HH_
#define HIGH_TO_ELC_HH_

// TODO: VisitValue does not need to be in sema.hh
#include "sema.hh"
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

    // When recursing, we would like to know the binding name as well as the expression
    VisitValue<std::pair<std::string, LCNodePtr>> v_elc_let;
};

#endif // HIGH_TO_ELC_HH_
