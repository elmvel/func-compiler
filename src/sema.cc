#include "sema.hh"

#include <fmt/core.h>
#include <fmt/std.h>

/*
TODO: I need to be able to represent function types
like `int -> int`, I need to be able to partially apply and
figure out through analysis that the types will work out
 */

auto fmt::formatter<Type>::format(Type c, fmt::v10::format_context& ctx) const
    -> format_context::iterator {
  string_view name = "<unknown>";
  switch (c) {
  case Type::Integer:  name="Integer";  break;
  case Type::String:   name="String";   break;
  }
  return formatter<std::string_view>::format(name, ctx);
}

void TreeSemaVisitor::visit(TreeSeqNode *node)
{
    for (auto& child : node->children) {
        child->accept(this);
    }
}

void TreeSemaVisitor::visit(TreeParamsNode *node)
{
    // TODO: SEMA for params
}

void TreeSemaVisitor::visit(TreeListNode *node)
{
    for (auto& child : node->children) {
        child->accept(this);
    }
}

void TreeSemaVisitor::visit(TreeBindingNode *node)
{
    // TODO: how should I treat parameters?
    // I treat the parameter list [a, ..., z] as just a TreeListNode.
    // However, this means I can't have special rules for traversing parameters,
    // which may have type annotations. Therefore, I need a special node for the
    // parameters list (which should probably just be sugar anyway...) so that I
    // can store type annotated information
    
    node->body->accept(this);
    if (node->next != nullptr) {
        node->next->accept(this);
    }
}

void TreeSemaVisitor::visit(TreeBinopNode *node)
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

void TreeSemaVisitor::visit(TreeApplyNode *node)
{
    node->func->accept(this);
    Type fn_type = v_type;

    // TODO: analyze that the type of the function is
    // consistent with the arguments passed...?
}

void TreeSemaVisitor::visit(TreeIdentNode *node)
{
    if (v_insert) {
        table.insert(Declaration {node->name, node->attr_type});
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
                    fmt::println("error: Binding `{}` has no type!", node->name);
                    abort();
                }
            } else {
                fmt::println("error: Could not find binding `{}`", node->name);
            }
        }
    }
}

void TreeSemaVisitor::visit(TreeIntegerNode *node)
{
    v_type = Type::Integer;
}

void TreeSemaVisitor::visit(TreeStringNode *node)
{
    v_type = Type::String;
}
