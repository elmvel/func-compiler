#ifndef ELC_TO_SIMPLE_HH_
#define ELC_TO_SIMPLE_HH_

#include <map>
#include <set>

#include "../common.hh"
#include "./lambda.hh"

struct LCSimpleVisitor : ILCVisitor
{
    LCSimpleVisitor()
        : output()
    {}
    
    virtual void visit(LCApplyNode *node);
    virtual void visit(LCLambdaNode *node);
    virtual void visit(LCDefNode *node);
    virtual void visit(LCLetNode *node);
    virtual void visit(LCCaseNode *node);
    virtual void visit(LCCaseArmNode *node);
    virtual void visit(LCIntNode *node);
    virtual void visit(LCBoolNode *node);
    virtual void visit(LCConstantNode *node);
    virtual void visit(LCDummyNode *node);

    int new_temp()
    {
        return temps++;
    }

    int last_temp()
    {
        return temps - 1;
    }

    std::string output;
    int temps = 0;
    std::map<std::string, int> bindings;
};

#endif // ELC_TO_SIMPLE_HH_
