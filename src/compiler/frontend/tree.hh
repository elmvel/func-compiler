#ifndef TREE_HH_
#define TREE_HH_

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "scanner.hh"
#include "types.hh"

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

struct TreeNode;
struct TreeSeqNode;
struct TreeParamsNode;
struct TreeListNode;
struct TreeBindingNode;
struct TreeBinopNode;
struct TreeApplyNode;
struct TreeMatchNode;
struct TreeMatchArmNode;
struct TreeIfNode;
struct TreeIdentNode;
struct TreeIntegerNode;
struct TreeStringNode;

struct ITreeVisitor
{
    virtual void visit(TreeSeqNode *node) = 0;
    virtual void visit(TreeParamsNode *node) = 0;
    virtual void visit(TreeListNode *node) = 0;
    virtual void visit(TreeBindingNode *node) = 0;
    virtual void visit(TreeBinopNode *node) = 0;
    virtual void visit(TreeApplyNode *node) = 0;
    virtual void visit(TreeMatchNode *node) = 0;
    virtual void visit(TreeMatchArmNode *node) = 0;
    virtual void visit(TreeIfNode *node) = 0;
    virtual void visit(TreeIdentNode *node) = 0;
    virtual void visit(TreeIntegerNode *node) = 0;
    virtual void visit(TreeStringNode *node) = 0;
};

struct TreeNode
{
    virtual ~TreeNode()
    {}

    virtual void accept(ITreeVisitor *visitor) = 0;
};

struct TreeSeqNode : TreeNode
{
    TreeSeqNode(std::vector<std::unique_ptr<TreeNode>> children)
        : children(std::move(children))
    {}

    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }

    std::vector<std::unique_ptr<TreeNode>> children;
};

struct TreeParamsNode : TreeNode
{
    TreeParamsNode(std::vector<std::unique_ptr<TreeNode>> children)
        : children(std::move(children))
    {}
    
    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }

    std::vector<std::unique_ptr<TreeNode>> children;
};

struct TreeListNode : TreeNode
{
    TreeListNode(std::vector<std::unique_ptr<TreeNode>> children)
        : children(std::move(children))
    {}

    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }

    std::vector<std::unique_ptr<TreeNode>> children;
};

struct TreeBindingNode : TreeNode
{
    TreeBindingNode(const std::string& id, std::unique_ptr<TreeNode>& body, std::unique_ptr<TreeNode>& params)
        : id(id), body(std::move(body)), params(std::move(params)), next(nullptr)
    {}
    
    virtual ~TreeBindingNode()
    {}

    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }
    
    std::string id;
    std::unique_ptr<TreeNode> body;
    std::unique_ptr<TreeNode> params;
    std::unique_ptr<TreeNode> next;

    // Attributes
    std::optional<TypePtr> attr_type;
};

struct TreeBinopNode : TreeNode
{
    TreeBinopNode(TokenType op, std::unique_ptr<TreeNode>& lhs, std::unique_ptr<TreeNode>& rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs))
    {}

    virtual ~TreeBinopNode()
    {}

    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }
    
    TokenType op;
    std::unique_ptr<TreeNode> lhs;
    std::unique_ptr<TreeNode> rhs;

    // Attributes
    std::optional<TypePtr> attr_type;
};

struct TreeApplyNode : TreeNode
{
    TreeApplyNode(std::unique_ptr<TreeNode> func, std::unique_ptr<TreeNode> arg)
        : func(std::move(func)), arg(std::move(arg))
    {}
    
    virtual ~TreeApplyNode()
    {}

    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }

    std::unique_ptr<TreeNode> func;
    std::unique_ptr<TreeNode> arg;

    // Attributes
    std::optional<TypePtr> attr_type;
};

struct TreeMatchNode : TreeNode
{
    TreeMatchNode(std::unique_ptr<TreeNode>& expr, std::vector<std::unique_ptr<TreeNode>> arms)
        : expr(std::move(expr)), arms(std::move(arms))
    {}

    virtual ~TreeMatchNode()
    {}

    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }

    std::unique_ptr<TreeNode> expr;
    std::vector<std::unique_ptr<TreeNode>> arms;
};

struct TreeMatchArmNode : TreeNode
{
    TreeMatchArmNode(std::unique_ptr<TreeNode>& pattern, std::unique_ptr<TreeNode>& body)
        : pattern(std::move(pattern)), body(std::move(body))
    {}

    virtual ~TreeMatchArmNode()
    {}

    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }

    std::unique_ptr<TreeNode> pattern;
    std::unique_ptr<TreeNode> body;
};

struct TreeIfNode : TreeNode
{
    TreeIfNode(std::unique_ptr<TreeNode>& cond, std::unique_ptr<TreeNode>& extrue, std::unique_ptr<TreeNode>& exfalse)
        : cond(std::move(cond)), extrue(std::move(extrue)), exfalse(std::move(exfalse))
    {}

    virtual ~TreeIfNode()
    {}

    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }

    std::unique_ptr<TreeNode> cond;
    std::unique_ptr<TreeNode> extrue;
    std::unique_ptr<TreeNode> exfalse;
};

struct TreeIdentNode : TreeNode
{
    TreeIdentNode(const std::string& name)
        : name(name)
    {}

    virtual ~TreeIdentNode()
    {}

    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }
    
    std::string name;

    // Attributes
    std::optional<TypePtr> attr_type;
};

struct TreeIntegerNode : TreeNode
{
    TreeIntegerNode(int value)
        : value(value)
    {}

    virtual ~TreeIntegerNode()
    {}

    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }
    
    int value;

    // Attributes
    std::optional<TypePtr> attr_type;
};

struct TreeStringNode : TreeNode
{
    TreeStringNode(const std::string& text)
        : text(text)
    {}

    virtual ~TreeStringNode()
    {}

    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }
    
    std::string text;

    // Attributes
    std::optional<TypePtr> attr_type;
};

#endif // TREE_HH_
