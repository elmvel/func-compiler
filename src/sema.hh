#include <string>
#include <unordered_map>

#include <fmt/core.h>

#include "common.hh"
#include "tree.hh"

struct TreeSemaVisitor : ITreeVisitor
{
    virtual void visit(TreeSeqNode *node)
    {
        for (auto& child : node->children) {
            child->accept(this);
        }
    }

    virtual void visit(TreeListNode *node)
    {
        for (auto& child : node->children) {
            child->accept(this);
        }
    }

    virtual void visit(TreeBindingNode *node)
    {
        // TODO: how should I treat parameters?
        node->body->accept(this);
        if (node->next != nullptr) {
            node->next->accept(this);
        }
    }

    virtual void visit(TreeBinopNode *node)
    {
        node->lhs->accept(this);
        Type ltype = v_type;

        node->rhs->accept(this);

        if (ltype != v_type) {
            // Could probably store a result in the visitor?
            fmt::println("error: Type mismatch in binary expression.");
            fmt::println("note: lhs was of type {} while rhs was of type {}.", ltype, v_type);
            abort();
        }
    }

    virtual void visit(TreeApplyNode *node)
    {
        node->func->accept(this);
        Type fn_type = v_type;

        // TODO: analyze that the type of the function is
        // consistent with the arguments passed...?
    }

    virtual void visit(TreeIdentNode *node)
    {
        auto it = symtab.find(node->name);
        if (it == symtab.end()) {
            fmt::println("error: Could not find name {}!", node->name);
            abort();
        }
        v_type = it->second;
    }

    virtual void visit(TreeIntegerNode *node)
    {
        v_type = Type::Integer;
    }

    virtual void visit(TreeStringNode *node)
    {
        v_type = Type::String;
    }

    // This implies to some degree that the order
    // of function definition matters, when it shouldn't.
    // Perhaps the symbol table should be computed in another pass?
    std::unordered_map<std::string, Type> symtab;
    Type v_type;
};
