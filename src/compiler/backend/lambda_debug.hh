#ifndef LAMBDA_DEBUG_HH_
#define LAMBDA_DEBUG_HH_

#include "../frontend/sema.hh"
#include "lambda.hh"

struct LCTraceVisitor : ILCVisitor
{
    virtual void visit(LCApplyNode *node);
    virtual void visit(LCLambdaNode *node);
    virtual void visit(LCDefNode *node);
    virtual void visit(LCLetNode *node);
    virtual void visit(LCCaseNode *node);
    virtual void visit(LCCaseArmNode *node);
    virtual void visit(LCIntNode *node);
    virtual void visit(LCBoolNode *node);
    virtual void visit(LCConstantNode *node);

    VisitValue<std::string> v_text;
};

#endif // LAMBDA_DEBUG_HH_
