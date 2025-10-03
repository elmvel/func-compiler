#include <cassert>

#include "sema.hh"
#include "types.hh"

#include <fmt/core.h>
#include <fmt/std.h>

/*
TODO: I need to be able to represent function types
like `int -> int`, I need to be able to partially apply and
figure out through analysis that the types will work out
 */

auto fmt::formatter<Type>::format(Type c, fmt::v10::format_context& ctx) const
    -> format_context::iterator {
    std::string name = "<unknown>";
    std::visit(overloaded {
        [&name](TypePrimitive prim)
        {
            switch (prim) {
            case TypePrimitive::Integer: name = std::string("int"); break;
            case TypePrimitive::String: name = std::string("string"); break;
            }
        },
        [&name](TypeFunction func)
        {
            assert(func.input != nullptr);
            assert(func.output != nullptr);
            std::string lhs = fmt::format("{}", *func.input);
            std::string rhs = fmt::format("{}", *func.output);
            name = fmt::format("{} -> {}", lhs, rhs);
        }
    }, c.storage);
    return formatter<std::string>::format(name, ctx);
}

void TreeSemaVisitor::visit(TreeSeqNode *node)
{
    for (auto& child : node->children) {
        child->accept(this);
    }
}

void TreeSemaVisitor::visit(TreeParamsNode *node)
{
    v_insert = true;
    for (auto& param : node->children) {
        param->accept(this);
    }
    v_insert = false;
}

void TreeSemaVisitor::visit(TreeListNode *node)
{
    for (auto& child : node->children) {
        child->accept(this);
    }
}

void TreeSemaVisitor::visit(TreeBindingNode *node)
{
    table.enter_scope();

    table.insert(Declaration {node->id, node->attr_type});
    
    if (node->params != nullptr) {
        node->params->accept(this);
    }
    
    node->body->accept(this);

    if (node->next != nullptr) {
        node->next->accept(this);
    }

    table.exit_scope();
}

void TreeSemaVisitor::visit(TreeBinopNode *node)
{
    node->lhs->accept(this);
    Type *ltype = v_type;

    node->rhs->accept(this);
    Type *rtype = v_type;

    if (*ltype != *rtype) {
        COMPILER_ERROR("Type mismatch in binary expression.");
        COMPILER_NOTE("lhs was of type `{}` while rhs was of type `{}`.", *ltype, *rtype);
        valid = false;
    }
}

void TreeSemaVisitor::visit(TreeApplyNode *node)
{
    node->func->accept(this);
    Type *fn_type = v_type;

    (void)fn_type;

    // TODO: analyze that the type of the function is
    // consistent with the arguments passed...?
}

void TreeSemaVisitor::visit(TreeIdentNode *node)
{
    if (v_insert) {
        table.insert(Declaration {node->name, node->attr_type});
        if (node->attr_type.has_value())
            v_type = *node->attr_type;
    } else {
        // First, see if it has an established type.
        if (node->attr_type.has_value()) {
            v_type = *node->attr_type;
        } else {
            auto decl = table.lookup(node->name);
            if (decl.has_value()) {
                if (decl->type.has_value()) {
                    v_type = *decl->type;
                } else {
                    COMPILER_ERROR("Binding `{}` has no type!", node->name);
                    valid = false;
                }
            } else {
                COMPILER_ERROR("Could not find binding `{}`", node->name);
                valid = false;
            }
        }
    }
}

void TreeSemaVisitor::visit(TreeIntegerNode *node)
{
    UNUSED(node);
    // For Dr. Z: Smart pointers
    // TODO: memory leak
    v_type = new Type(TypePrimitive::Integer);
}

void TreeSemaVisitor::visit(TreeStringNode *node)
{
    UNUSED(node);
    // TODO: memory leak
    v_type = new Type(TypePrimitive::String);
}
