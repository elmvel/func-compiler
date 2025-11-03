#include "lambda_debug.hh"

#include <fmt/core.h>
#include <fmt/std.h>

// Trace functions are careless with strings for simplicity
void LCTraceVisitor::visit(LCApplyNode *node)
{
    node->fun->accept(this);
    std::string traced_fun = v_text.read_asserted();
    node->arg->accept(this);
    std::string traced_arg = v_text.read_asserted();
    v_text.write(fmt::format("({} {})", traced_fun, traced_arg));
}

void LCTraceVisitor::visit(LCLambdaNode *node)
{
    node->body->accept(this);
    std::string traced_body = v_text.read_asserted();
    v_text.write(fmt::format("(\\{} . {})", node->param, traced_body));
}

void LCTraceVisitor::visit(LCDefNode *node)
{
    // @remove
    // node->var->accept(this);
    // std::string traced_var = v_text.read_asserted();
    node->body->accept(this);
    std::string traced_body = v_text.read_asserted();
    v_text.write(fmt::format("{} = {}", node->var, traced_body));
}

void LCTraceVisitor::visit(LCLetNode *node)
{
    std::string defs = "";
    for (size_t i = 0; i < node->definitions.size(); ++i) {
        if (i > 0) defs += ";";
        node->definitions[i]->accept(this);
        std::string def = v_text.read_asserted();
        defs += def;
    }
    node->expr->accept(this);
    std::string traced_expr = v_text.read_asserted();
    v_text.write(fmt::format("(let {} in {})", defs, traced_expr));
}

void LCTraceVisitor::visit(LCIntNode *node)
{
    v_text.write(fmt::format("{}", node->value));
}

void LCTraceVisitor::visit(LCBoolNode *node)
{
    v_text.write(fmt::format("{}", node->value));
}

void LCTraceVisitor::visit(LCConstantNode *node)
{
    v_text.write(fmt::format("{}", node->name));
}
