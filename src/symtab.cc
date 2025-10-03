#include "symtab.hh"

TreeSymtabVisitor::TreeSymtabVisitor()
    : table()
{}

void TreeSymtabVisitor::visit(TreeSeqNode *node)
{
    for (auto& child : node->children) {
        child->accept(this);
    }
}

void TreeSymtabVisitor::visit(TreeParamsNode *node)
{
    for (auto& child : node->children) {
        child->accept(this);
    }
}

void TreeSymtabVisitor::visit(TreeListNode *node)
{
    for (auto& child : node->children) {
        child->accept(this);
    }
}

void TreeSymtabVisitor::visit(TreeBindingNode *node)
{
    // Insert this `let x = ...` into the symbol table
    table.insert(Declaration {node->id, node->attr_type});

    table.enter_scope();
    
    if (node->params != nullptr) {
        node->params->accept(this);
    }

    node->body->accept(this);

    if (node->next != nullptr) {
        node->next->accept(this);
    }

    table.exit_scope();
}

void TreeSymtabVisitor::visit(TreeBinopNode *node)
{
    node->lhs->accept(this);
    node->rhs->accept(this);
}

void TreeSymtabVisitor::visit(TreeApplyNode *node)
{
    node->func->accept(this);
    node->arg->accept(this);
}

void TreeSymtabVisitor::visit(TreeIdentNode *node)
{
    (void)node;
}

void TreeSymtabVisitor::visit(TreeIntegerNode *node)
{
    (void)node;
}

void TreeSymtabVisitor::visit(TreeStringNode *node)
{
    (void)node;
}
