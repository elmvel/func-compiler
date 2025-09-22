#ifndef TREE_H_
#define TREE_H_

#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "scanner.hh"

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
struct TreeListNode;
struct TreeBindingNode;
struct TreeBinopNode;
struct TreeApplyNode;
struct TreeIdentNode;
struct TreeIntegerNode;
struct TreeStringNode;

struct ITreeVisitor
{
    virtual void visit(TreeSeqNode *node) = 0;
    virtual void visit(TreeListNode *node) = 0;
    virtual void visit(TreeBindingNode *node) = 0;
    virtual void visit(TreeBinopNode *node) = 0;
    virtual void visit(TreeApplyNode *node) = 0;
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
    TreeSeqNode(const std::vector<TreeNode *> children)
        : children(children)
    {}

    virtual ~TreeSeqNode()
    {
        for (auto& child : children) {
            delete child;
        }
    }
    
    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }

    std::vector<TreeNode *> children;
};

struct TreeListNode : TreeNode
{
    TreeListNode(const std::vector<TreeNode *> children)
        : children(children)
    {}

    virtual ~TreeListNode()
    {
        for (auto& child : children) {
            delete child;
        }
    }
    
    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }

    std::vector<TreeNode *> children;
};

struct TreeBindingNode : TreeNode
{
    TreeBindingNode(const std::string& id, TreeNode *body, TreeNode *params = nullptr)
        : id(id), body(body), params(params), next(nullptr)
    {}
    
    virtual ~TreeBindingNode()
    {
        delete body;
        delete params;
        delete next;
    }

    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }
    
    std::string id;
    TreeNode *body;
    TreeNode *params;
    TreeNode *next;
};

struct TreeBinopNode : TreeNode
{
    TreeBinopNode(TokenType op, TreeNode *lhs, TreeNode *rhs)
        : op(op), lhs(lhs), rhs(rhs)
    {}

    virtual ~TreeBinopNode()
    {
        delete lhs;
        delete rhs;
    }

    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }
    
    TokenType op;
    TreeNode *lhs;
    TreeNode *rhs;
};

struct TreeApplyNode : TreeNode
{
    TreeApplyNode(TreeNode *func, TreeNode *arg)
        : func(func), arg(arg)
    {}
    
    virtual ~TreeApplyNode()
    {
        delete func;
        delete arg;
    }

    virtual void accept(ITreeVisitor *visitor)
    {
        visitor->visit(this);
    }

    TreeNode *func;
    TreeNode *arg;
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
};

#endif // TREE_H_
