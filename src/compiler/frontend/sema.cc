#include <cassert>

#include "sema.hh"
#include "types.hh"

#include <fmt/core.h>
#include <fmt/std.h>

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
    for (auto& param : node->children) {
        v_insert.write(true);
        param->accept(this);
    }
}

void TreeSemaVisitor::visit(TreeListNode *node)
{
    for (auto& child : node->children) {
        child->accept(this);
    }
}

void TreeSemaVisitor::visit(TreeBindingNode *node)
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

void TreeSemaVisitor::visit(TreeBinopNode *node)
{
    node->lhs->accept(this);
    TypePtr ltype = v_type.read_or(std::make_shared<Type>(TypePrimitive::Integer));

    node->rhs->accept(this);
    TypePtr rtype = v_type.read_or(std::make_shared<Type>(TypePrimitive::Integer));

    if (*ltype != *rtype) {
        COMPILER_ERROR("Type mismatch in binary expression.");
        COMPILER_NOTE("lhs was of type `{}` while rhs was of type `{}`.", *ltype, *rtype);
        valid = false;

        // Keep v_type invalid
        return;
    }

    v_type.write(ltype);
}

void TreeSemaVisitor::visit(TreeApplyNode *node)
{
    v_type.erase();
    node->func->accept(this);

    // We can't examine a function application that is already invalid
    if (!v_type.is_valid()) return;
    
    TypePtr fn_type = v_type.read_asserted();

    node->arg->accept(this);
    TypePtr arg_type = v_type.read_asserted();

    assert(fn_type->get_input().has_value());
    TypePtr expected = *fn_type->get_input();
    TypePtr got = arg_type;
    if (*got != *expected) {
        COMPILER_ERROR("Type mismatch in function application.");
        COMPILER_NOTE("Function accepted `{}` but was supplied `{}` instead.",
                      *expected, *got);
        valid = false;
    }

    auto ret_type = fn_type->get_output();
    assert(ret_type.has_value());
    v_type.write(*ret_type);
}

void TreeSemaVisitor::visit(TreeMatchNode *node)
{
    v_type.erase();
    node->expr->accept(this);
    TypePtr expr_type = v_type.read_asserted();

    table.enter_scope();

    // First pass, check to see if all the cases match types with the examined expr
    match_arm_yield_body_type = false;
    for (auto& arm : node->arms) {
        v_type.write(expr_type);
        arm->accept(this);
        TypePtr arm_type = v_type.read_asserted();

        if (*expr_type != *arm_type) {
            COMPILER_ERROR("Type mismatch in match case.");
            COMPILER_NOTE("Match scrutinee was of type `{}`, but had an arm of `{}` instead.",
                          *expr_type, *arm_type);
            valid = false;
        }
    }

    // Then, check to see that all arm bodies yield the same type

    // TODO: decide how to approach semantic analysis on arm bodies,
    // or should we simply postpone it due to the unclear nature of
    // type checking here.
    
    // match_arm_yield_body_type = true;
    // TypePtr common_type = nullptr;
    // for (auto& arm : node->arms) {
    //     arm->accept(this);
    //     TypePtr body_type = v_type.read_asserted();

    //     if (!common_type) {
    //         common_type = body_type;
    //         continue;
    //     }

    //     if (*common_type != *body_type) {
    //         COMPILER_ERROR("Type mismatch in match case.");
    //         COMPILER_NOTE("Match result was of type `{}`, but one body was of type `{}` instead.",
    //                       *common_type, *body_type);
    //         valid = false;
    //     }
    // }

    table.exit_scope();
}

void TreeSemaVisitor::visit(TreeMatchArmNode *node)
{
    if (match_arm_yield_body_type) {
        // TODO: see above
        // node->body->accept(this);
    } else {
        v_insert.write(true);
        node->pattern->accept(this);
    }
}

void TreeSemaVisitor::visit(TreeIfNode *node)
{
    node->cond->accept(this);
    TypePtr cond_type = v_type.read_or(std::make_shared<Type>(TypePrimitive::Integer));
    TypePrimitive* cond_prim = std::get_if<TypePrimitive>(&cond_type->storage);
    if (cond_prim == nullptr)
    {
        COMPILER_ERROR("Condition was not a primitive! Condition must result in an integer!");
        valid = false;
        return;
    }
    if (*cond_prim != TypePrimitive::Integer)
    {
        COMPILER_ERROR("Condition must be a string!");
        COMPILER_NOTE("Condition was of type `{}`.", *cond_type);
        valid = false;
        return;
    }

    node->extrue->accept(this);
    TypePtr extrue_type = v_type.read_or(std::make_shared<Type>(TypePrimitive::Integer));
    node->exfalse->accept(this);
    TypePtr exfalse_type = v_type.read_or(std::make_shared<Type>(TypePrimitive::Integer));
    if (*extrue_type != *exfalse_type)
    {
        COMPILER_ERROR("If expression branch types must match!");
        COMPILER_NOTE("True branch was of type `{}` while false was of type `{}`.", *extrue_type, *exfalse_type);
        valid = false;
        return;
    }

    v_type.write(extrue_type);
}

void TreeSemaVisitor::visit(TreeIdentNode *node)
{
    if (v_insert.is_valid() && v_insert.read_asserted()) {
        table.insert(Declaration {node->name, node->attr_type});
        if (node->attr_type.has_value())
            v_type.write(*node->attr_type);
    } else {
        // First, see if it has an established type.
        if (node->attr_type.has_value()) {
            v_type.write(*node->attr_type);
        } else {
            auto decl = table.lookup(node->name);
            if (decl.has_value()) {
                if (decl->arity.has_value())
                    v_arity.write(*decl->arity);

                if (decl->type.has_value()) {
                    v_type.write(*decl->type);
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
    v_type.write(std::make_shared<Type>(TypePrimitive::Integer));
}

void TreeSemaVisitor::visit(TreeStringNode *node)
{
    UNUSED(node);
    v_type.write(std::make_shared<Type>(TypePrimitive::String));
}
