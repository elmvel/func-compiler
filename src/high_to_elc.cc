#include "high_to_elc.hh"

#include "common.hh"

void TreeToELCVisitor::visit(TreeSeqNode *node)
{
    (void)node;
    COMPILER_ERROR_TERM("TODO: High->ELC for TreeSeqNode");
}

void TreeToELCVisitor::visit(TreeParamsNode *node)
{
    (void)node;
    COMPILER_ERROR_TERM("TODO: High->ELC for TreeParamsNode");
}

void TreeToELCVisitor::visit(TreeListNode *node)
{
    (void)node;
    COMPILER_ERROR_TERM("TODO: High->ELC for TreeListNode");
}

void TreeToELCVisitor::visit(TreeBindingNode *node)
{
    (void)node;
    COMPILER_ERROR_TERM("TODO: High->ELC for TreeBindingNode");
}

std::string token_to_builtin(TokenType token)
{
    switch (token) {
    case TokenType::Add: return "ADD";
    case TokenType::Sub: return "SUB";
    case TokenType::Mul: return "MUL";
    case TokenType::Div: return "DIV";
    default: COMPILER_ERROR_TERM("Unknown builtin");
    }
}

void TreeToELCVisitor::visit(TreeBinopNode *node)
{
    node->lhs->accept(this);
    LCNodePtr elc_lhs = v_elc.read_asserted();
    node->rhs->accept(this);
    LCNodePtr elc_rhs = v_elc.read_asserted();
    std::string builtin = token_to_builtin(node->op);

    LCNodePtr elc_fn = std::make_shared<LCConstantNode>(builtin);
    LCNodePtr curried = std::make_shared<LCApplyNode>(elc_fn, elc_lhs);
    v_elc.write(std::make_shared<LCApplyNode>(curried, elc_rhs));
}

void TreeToELCVisitor::visit(TreeApplyNode *node)
{
    node->func->accept(this);
    LCNodePtr elc_fun = v_elc.read_asserted();
    node->arg->accept(this);
    LCNodePtr elc_arg = v_elc.read_asserted();
    v_elc.write(std::make_shared<LCApplyNode>(elc_fun, elc_arg));
}

void TreeToELCVisitor::visit(TreeMatchNode *node)
{
    (void)node;
    COMPILER_ERROR_TERM("TODO: High->ELC for TreeMatchNode");
}

void TreeToELCVisitor::visit(TreeMatchArmNode *node)
{
    (void)node;
    COMPILER_ERROR_TERM("TODO: High->ELC for TreeMatchArmNode");
}

void TreeToELCVisitor::visit(TreeIdentNode *node)
{
    v_elc.write(std::make_shared<LCConstantNode>(node->name));
}

void TreeToELCVisitor::visit(TreeIntegerNode *node)
{
    v_elc.write(std::make_shared<LCIntNode>(node->value));
}

void TreeToELCVisitor::visit(TreeStringNode *node)
{
    (void)node;
    COMPILER_ERROR_TERM("TODO: High->ELC for TreeStringNode");
}
