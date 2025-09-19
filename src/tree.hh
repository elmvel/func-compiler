#ifndef TREE_H_
#define TREE_H_

#include <optional>
#include <string>
#include <string_view>
#include <variant>

/*
  Since so many of these will be created,
  the file path is a non-owning string view.
 */
struct SourceLocation
{
    SourceLocation(const std::string_view& file_path, int lineno, int colno)
        : file_path(file_path), lineno(lineno), colno(colno)
    {}
    
    std::string_view file_path;
    int lineno;
    int colno;
};

enum class NodeKind
{
    Binding,
    Expr
};

enum class ExprKind
{
    // Primitives
    Ident,
    Integer,
    String,

    // Arithmetic
    Add,
    Sub,
    Mul,
    Div,

    // Functions
    Funcall,

    // List
    List,
    Cons,

    // Branching
    IfElse,
    Match,
};

#define TREE_MAX_CHILDREN 3

struct TreeNode
{
    using value = TreeNode;
    using pointer = value*;
    using reference = value&;
    using attr_type = std::variant<int, std::string>;

    TreeNode()
        : expr_kind({}), attr({})
    {
        std::fill(children, children+TREE_MAX_CHILDREN, nullptr);
    }
    
    NodeKind kind;
    std::optional<ExprKind> expr_kind;
    attr_type attr;
    TreeNode *children[TREE_MAX_CHILDREN];

    pointer& left();
    pointer& right();
    pointer& middle();
    pointer& next();
};

// Very unsafe, will need to consider better allocation options
TreeNode *new_binding_node(const std::string& id, TreeNode *body, TreeNode *params = nullptr);
TreeNode *new_expr_node(ExprKind expr_kind, const TreeNode::attr_type& attr);

#endif // TREE_H_
