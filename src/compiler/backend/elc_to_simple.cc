#include "./elc_to_simple.hh"

#include <fmt/core.h>

#include "../common.hh"

void LCSimpleVisitor::visit(LCApplyNode *node)
{
    node->fun->accept(this);
    int temp_fun = last_temp();
    node->arg->accept(this);
    int temp_arg = last_temp();
    output.append(fmt::format("    auto __{} = std::make_shared<SLCNode>(APP, SLCNodeApp {{ __{}, __{} }});\n", new_temp(), temp_fun, temp_arg));
}

void LCSimpleVisitor::visit(LCLambdaNode *node)
{
    node->body->accept(this);
    int temp = last_temp();
    output.append(fmt::format("    auto __{} = std::make_shared<SLCNode>(LAM, SLCNodeLam {{\"{}\", __{} }});\n", new_temp(), node->param, temp));
}

void LCSimpleVisitor::visit(LCDefNode *node)
{
    UNUSED(node);
    INTERNAL_ERROR("TODO: LCDefNode ");
}

void LCSimpleVisitor::visit(LCLetNode *node)
{
    for (auto& n : node->definitions)
    {
        LCDefNode *def = static_cast<LCDefNode *>(n.get());
        def->body->accept(this);
        bindings[def->var] = last_temp();
    }
    node->expr->accept(this);
}

void LCSimpleVisitor::visit(LCCaseNode *node)
{
    UNUSED(node);
    INTERNAL_ERROR("TODO: LCCaseNode ");
}

void LCSimpleVisitor::visit(LCCaseArmNode *node)
{
    UNUSED(node);
    INTERNAL_ERROR("TODO: LCCaseArmNode ");
}

void LCSimpleVisitor::visit(LCIntNode *node)
{
    output.append(fmt::format("    auto __{} = std::make_shared<SLCNode>(INT, {});\n", new_temp(), node->value));
}

void LCSimpleVisitor::visit(LCBoolNode *node)
{
    UNUSED(node);
    INTERNAL_ERROR("TODO: LCBoolNode ");
}

void LCSimpleVisitor::visit(LCConstantNode *node)
{
    // TODO: not the most performant nor best way to write this.
    std::set<std::string> bifs {
        "ADD",
        "SUB",
        "MUL",
        "DIV",
        "EQU",
        "IF",
    };

    std::string type {};

    if (bifs.count(node->name) > 0)
    {
        type = "BIF";
    }
    else
    {
        type = "VAR";
    }

    auto it = bindings.find(node->name);
    if (it != bindings.end())
    {
        output.append(fmt::format("    auto __{} = __{};\n", new_temp(), it->second));
    }
    else
    {
        output.append(fmt::format("    auto __{} = std::make_shared<SLCNode>({}, \"{}\");\n", new_temp(), type, node->name));
    }
}

void LCSimpleVisitor::visit(LCDummyNode *node)
{
    UNUSED(node);
    INTERNAL_ERROR("TODO: LCDummyNode ");
}

