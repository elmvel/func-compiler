#include "lambda_lifting.hh"

#include <algorithm>

#include "../common.hh"

LCNodePtr LCSupercombinatorVisitor::visit(LCApplyNode *node)
{
    LCNodePtr fun = node->fun->accept(this);
    LCNodePtr arg = node->arg->accept(this);
    return std::make_shared<LCApplyNode>(fun, arg);
}

LCNodePtr LCSupercombinatorVisitor::visit(LCLambdaNode *node)
{
    if (node->count_inner_lambdas() == 1) {
        // Only 1 inner lambda (the lambda itself), so we can lift

        /*
          First, we need to find all of the free variables and wrap
          the lambda with lambdas that capture the free variables.
        */

        FreeVarsEnv env;

        // Supercombinators should be bound.
        for (auto& [sc_name, _] : supercombinators) {
            env.bound_vars.insert(sc_name);
        }
        
        node->find_free_vars(env);
        LCNodePtr lambda = std::make_shared<LCLambdaNode>(*node);

        using FreeVar = std::pair<std::string, LevelNo>;
        std::vector<FreeVar> sorted_frees;
        for (auto& [var, lno] : env.free_vars) {
            sorted_frees.push_back({var, lno});
        }

        // Sort the free variables in increasing lexical scope
        // for the best chance of supercombinator elimination.
        std::sort(sorted_frees.begin(), sorted_frees.end(), [](FreeVar a, FreeVar b) {
            return a.second < b.second;
        });

        // Wrap the lambda with the free variables
        for (int i = sorted_frees.size() - 1; i >= 0; --i) {
            const FreeVar& fv = sorted_frees[i];
            lambda = std::make_shared<LCLambdaNode>(fv.first, lambda);
        }

        std::string sc_name = fmt::format("__SC_{}", counter++);
        supercombinators[sc_name] = lambda;

        /*
          In place of the Lambda, we want the supercombinator applied
          to all of the free variables that we collected.
        */

        LCNodePtr sc = std::make_shared<LCConstantNode>(sc_name);
        LCNodePtr app = sc;
        // Go forwards to apply in the correct order
        for (int i = 0; i < (int)sorted_frees.size(); ++i) {
            const FreeVar& fv = sorted_frees[i];
            LCNodePtr fv0 = std::make_shared<LCConstantNode>(fv.first);
            app = std::make_shared<LCApplyNode>(app, fv0);
            lambda = std::make_shared<LCLambdaNode>(fv.first, lambda);
        }

        return app;
    } else {
        // Cannot lift quite yet
        LCNodePtr body = node->body->accept(this);
        return std::make_shared<LCLambdaNode>(node->param, body);
    }
}

LCNodePtr LCSupercombinatorVisitor::visit(LCDefNode *node)
{
    LCNodePtr body = node->body->accept(this);
    return std::make_shared<LCDefNode>(node->var, body);
}

LCNodePtr LCSupercombinatorVisitor::visit(LCLetNode *node)
{
    if (node->level == 0 && node->count_inner_lambdas() == 0) {
        // Toplevel of supercombinators => lift as supercombinators
        for (auto& def : node->definitions) {
            LCDefNode *def_ptr = static_cast<LCDefNode *>(def.get());
            LCNodePtr body = def_ptr->body->accept(this);
            supercombinators[def_ptr->var] = body;
        }

        // After lifting the definitions as supercombinators, we
        // should be left with just the inner expression.
        return node->expr->accept(this);
    } else {
        std::vector<LCNodePtr> definitions;
        for (auto& def : node->definitions) {
            definitions.emplace_back(def->accept(this));
        }
        LCNodePtr expr = node->expr->accept(this);
        return std::make_shared<LCLetNode>(definitions, expr, node->recursive);
    }
}

LCNodePtr LCSupercombinatorVisitor::visit(LCCaseNode *node)
{
    UNUSED(node);
    INTERNAL_ERROR("TODO: support pattern matching here");
}

LCNodePtr LCSupercombinatorVisitor::visit(LCCaseArmNode *node)
{
    UNUSED(node);
    INTERNAL_ERROR("TODO: support pattern matching here");
}

LCNodePtr LCSupercombinatorVisitor::visit(LCIntNode *node)
{
    UNUSED(node);
    return std::make_shared<LCIntNode>(*node);
}

LCNodePtr LCSupercombinatorVisitor::visit(LCBoolNode *node)
{
    return std::make_shared<LCBoolNode>(*node);
}

LCNodePtr LCSupercombinatorVisitor::visit(LCConstantNode *node)
{
    return std::make_shared<LCConstantNode>(*node);
}

LCNodePtr LCSupercombinatorVisitor::visit(LCDummyNode *node)
{
    return std::make_shared<LCDummyNode>(*node);
}

std::optional<LCNodePtr> eta_reduction(const LCSupercombinatorMap& supercombinators, LCNodePtr node)
{
    LCLambdaNode *lambda = dynamic_cast<LCLambdaNode *>(node.get());
    if (lambda == nullptr) return {};

    LCApplyNode *app = dynamic_cast<LCApplyNode *>(lambda->body.get());
    if (app == nullptr) return {};

    // Don't need to check if app->fun is a function, as
    // it wouldn't be applied to an argument anyway.
    
    FreeVarsEnv env;

    // Supercombinators should be bound.
    for (auto& [sc_name, _] : supercombinators) {
        env.bound_vars.insert(sc_name);
    }
    
    app->fun->find_free_vars(env);

    if (env.free_vars.find(lambda->param) != env.free_vars.end()) {
        // The formal parameter was free.
        return {};
    } else {
        // The formal parameter was not free.
        // We are eligible for eta-reduction.
        return app->fun;
    }
}
