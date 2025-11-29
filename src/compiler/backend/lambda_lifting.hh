#ifndef LAMBDA_LIFTING_HH_
#define LAMBDA_LIFTING_HH_

#include <unordered_map>
#include <string>
#include <optional>

#include "../frontend/sema.hh"
#include "lambda.hh"

struct LCSupercombinatorVisitor : ILCTransformVisitor
{
    virtual LCNodePtr visit(LCApplyNode *node);
    virtual LCNodePtr visit(LCLambdaNode *node);
    virtual LCNodePtr visit(LCDefNode *node);
    virtual LCNodePtr visit(LCLetNode *node);
    virtual LCNodePtr visit(LCCaseNode *node);
    virtual LCNodePtr visit(LCCaseArmNode *node);
    virtual LCNodePtr visit(LCIntNode *node);
    virtual LCNodePtr visit(LCBoolNode *node);
    virtual LCNodePtr visit(LCConstantNode *node);
    virtual LCNodePtr visit(LCDummyNode *node);

    LCSupercombinatorMap supercombinators;
    int counter = 0;
};

std::optional<LCNodePtr> eta_reduction(const LCSupercombinatorMap& supercombinators, LCNodePtr node);

#endif // LAMBDA_LIFTING_HH_
