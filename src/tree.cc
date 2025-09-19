#include "tree.hh"

TreeNode::pointer& TreeNode::left()
{
    return children[0];
}

TreeNode::pointer& TreeNode::right()
{
    return children[1];
}

TreeNode::pointer& TreeNode::middle()
{
    return children[2];
}

TreeNode::pointer& TreeNode::next()
{
    return middle();
}

TreeNode *new_binding_node(const std::string& id, TreeNode *body, TreeNode *params)
{
    TreeNode *node = new TreeNode();
    node->kind = NodeKind::Binding;
    node->attr = id;
    node->left() = body;
    node->right() = params;
    return node;
}

TreeNode *new_expr_node(ExprKind expr_kind, const TreeNode::attr_type& attr)
{
    TreeNode *node = new TreeNode();
    node->kind = NodeKind::Expr;
    node->expr_kind = expr_kind;
    node->attr = attr;
    return node;
}
