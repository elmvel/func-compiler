#include "tree.hh"

struct TreeTraceVisitor : ITreeVisitor
{
    virtual void visit(TreeSeqNode *node)
    {
        std::string local_text = "";
        for (auto& child : node->children) {
            child->accept(this);
            local_text.append(text);
            local_text.append("\n\n");
        }
        text = local_text;
    }

    virtual void visit(TreeParamsNode *node)
    {
        std::string local_text = "";
        local_text.push_back('[');
        for (size_t i = 0; i < node->children.size(); ++i) {
            if (i != 0) {
                local_text.append(", ");
            }
            node->children[i]->accept(this);
            local_text.append(text);
        }
        local_text.push_back(']');
        text = local_text;
    }

    virtual void visit(TreeListNode *node)
    {
        std::string local_text = "";
        local_text.push_back('[');
        for (size_t i = 0; i < node->children.size(); ++i) {
            if (i != 0) {
                local_text.append(", ");
            }
            node->children[i]->accept(this);
            local_text.append(text);
        }
        local_text.push_back(']');
        text = local_text;
    }

    virtual void visit(TreeBindingNode *node)
    {
        std::string local_text = "Let(";

        local_text.append(node->id);
        if (node->params != nullptr) {
            local_text.append(", ");
            node->params->accept(this);
            local_text.append(text);
        }
        local_text.append(")\n");

        level += 1;
        for (int i = 0; i < level; ++i) {
            local_text.append("    ");
        }
        node->body->accept(this);
        local_text.append(text);
        level -= 1;

        if (node->next != nullptr) {
            local_text.append(",\n");
            for (int i = 0; i < level; ++i) {
                local_text.append("    ");
            }
            node->next->accept(this);
            local_text.append(text);
        }

        text = local_text;
    }

    virtual void visit(TreeBinopNode *node)
    {
        std::string local_text = fmt::format("{}(", node->op);
        node->lhs->accept(this);
        local_text.append(text);
        local_text.append(", ");
        node->rhs->accept(this);
        local_text.append(text);
        local_text.append(")");
        
        text = local_text;
    }

    virtual void visit(TreeApplyNode *node)
    {
        std::string local_text = "Apply(";

        node->func->accept(this);
        local_text.append(text);
        local_text.append(", ");
        node->arg->accept(this);
        local_text.append(text);
        local_text.append(")");
        
        text = local_text;
    }

    virtual void visit(TreeMatchNode *node)
    {
        std::string local_text = "Match(";

        node->expr->accept(this);
        local_text.append(text);
        for (auto& node : node->arms) {
            local_text.append(", ");
            node->accept(this);
            local_text.append(text);
        }
        local_text.append(")");
        

        text = local_text;
    }

    virtual void visit(TreeMatchArmNode *node)
    {
        std::string local_text = "";
        
        node->pattern->accept(this);
        local_text.append(text);
        local_text.append(" => ");
        node->body->accept(this);
        local_text.append(text);

        text = local_text;
    }

    virtual void visit(TreeIdentNode *node)
    {
        text = node->name;
    }

    virtual void visit(TreeIntegerNode *node)
    {
        std::string local_text = fmt::format("{}", node->value);
        text = local_text;
    }

    virtual void visit(TreeStringNode *node)
    {
        text = fmt::format("\"{}\"", node->text);
    }

    std::string text;
    int level = 0;
};
