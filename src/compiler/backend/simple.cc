#include <cassert>
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <functional>
#include <unordered_map>

// Basic Graph Reduction
// Refernced from Simon Peyton Jones

enum SLCNodeEnum
{
    INT,
    VAR,
    APP,
    LAM,
    BIF,
};

struct SLCNode;

// In more sophisticated implementations, these may be shared.
// Ultimately, nodes will be lost in the heap and have to be
// garbage collected. Here, we avoid it lazily by using reference counting.
using SLCNodePtr = std::shared_ptr<SLCNode>;

struct SLCNodeApp
{
    SLCNodeApp(SLCNodePtr fun, SLCNodePtr arg)
        : fun(std::move(fun)), arg(std::move(arg))
    {}
    
    SLCNodePtr fun;
    SLCNodePtr arg;
};

struct SLCNodeLam
{
    SLCNodeLam(const std::string& param, SLCNodePtr body)
        : param(param), body(body)
    {}
    
    std::string param;
    SLCNodePtr body;
};

using SLCNodeData = std::variant<int, std::string, SLCNodeApp, SLCNodeLam>;
struct SLCNode
{
    SLCNode(SLCNodeEnum tag, SLCNodeData data)
        : tag(tag), data(data)
    {}
    
    SLCNodeEnum tag;
    SLCNodeData data;
};

// This is for optimizing the grabbing of arguments per page 203.
struct SLCStack
{
    void save()
    {
        dump.push_back(c);
        c = {};
    }

    void restore()
    {
        c = dump.back();
        dump.pop_back();
    }

    SLCNodePtr& at(int i)
    {
        return c[i];
    }

    void push(const SLCNodePtr& node)
    {
        c.push_back(node);
    }

    int depth()
    {
        return c.size();
    }
    
    std::vector<SLCNodePtr> c;
    std::vector<std::vector<SLCNodePtr>> dump;
};

// NOTE: may need to reference Chapter 11 to correctly implement finding the next redex.

void eval(SLCStack& stack, SLCNodePtr expr);

std::unordered_map<std::string, std::function<SLCNodePtr(SLCNodePtr, SLCNodePtr)>> builtin_map = {
    std::pair {"ADD", [](SLCNodePtr lhs, SLCNodePtr rhs) {
        return std::make_shared<SLCNode>(INT, std::get<INT>(lhs->data) + std::get<INT>(rhs->data));
    }},
    std::pair {"SUB", [](SLCNodePtr lhs, SLCNodePtr rhs) {
        return std::make_shared<SLCNode>(INT, std::get<INT>(lhs->data) - std::get<INT>(rhs->data));
    }},
    std::pair {"MUL", [](SLCNodePtr lhs, SLCNodePtr rhs) {
        return std::make_shared<SLCNode>(INT, std::get<INT>(lhs->data) * std::get<INT>(rhs->data));
    }},
    std::pair {"DIV", [](SLCNodePtr lhs, SLCNodePtr rhs) {
        return std::make_shared<SLCNode>(INT, std::get<INT>(lhs->data) / std::get<INT>(rhs->data));
    }},
    std::pair {"EQU", [](SLCNodePtr lhs, SLCNodePtr rhs) {
        return std::make_shared<SLCNode>(INT, std::get<INT>(lhs->data) == std::get<INT>(rhs->data));
    }},
};

// Display a node to stdout
void print(SLCNodePtr node)
{
    switch (node->tag)
    {
        case INT: printf("%d", std::get<INT>(node->data)); break;
        case VAR: printf("%s", std::get<VAR>(node->data).c_str()); break;
        case APP: {
            auto& fun = std::get<APP>(node->data).fun;
            auto& arg = std::get<APP>(node->data).arg;
            printf("(");
            print(fun);
            printf(" ");
            print(arg);
            printf(")");
        } break;
        case LAM: {
            auto& param = std::get<LAM>(node->data).param;
            auto& body = std::get<LAM>(node->data).body;
            printf("(\\%s. ", param.c_str());
            print(body);
            printf(")");
        } break;
        case BIF: {
            auto& name = std::get<std::string>(node->data);
            printf("%s", name.c_str());
        } break;
    }
}
// void print(SLCNodePtr node)
// {
//     switch (node->tag)
//     {
//         case INT: printf("%d\n", std::get<INT>(node->data)); break;
//         case VAR:
//         case APP:
//         case LAM:
//         case BIF:
//             assert(0 && "Unsupported node in print");
//     }
// }

// *Recursively* copy body, substituting variables for a given value.
// This is precisely the operation that supercombinator compilation
// aims to optimize, where instead of recursively copying trees, we
// follow a set of instructions in constant time to construct a body.
//
// Of course, "constant time" with a large body will be a large amount
// of instructions, but we won't be jumping around in memory and performing
// a check for the parameter at each and every variable node.
SLCNodePtr instantiate(SLCNodePtr body, const std::string& param, SLCNodePtr value)
{
    switch (body->tag)
    {
        case INT: return body;
        case VAR: {
            const std::string& var = std::get<VAR>(body->data);
            if (var == param)
                return value;
            else
                return body;
        }
        case APP: {
            SLCNodePtr& fun = std::get<APP>(body->data).fun;
            SLCNodePtr& arg = std::get<APP>(body->data).arg;
            return std::make_shared<SLCNode>(APP, SLCNodeApp {
                    instantiate(fun, param, value), instantiate(arg, param, value)});
        }
        case LAM: {
            const std::string& l_param = std::get<LAM>(body->data).param;
            SLCNodePtr& l_body = std::get<LAM>(body->data).body;
            if (l_param == param)
                return body;
            else
                return std::make_shared<SLCNode>(
                    LAM, SLCNodeLam {l_param, instantiate(l_body, param, value)});
        }
        case BIF: return body;
    }
    assert(0 && "Unreachable");
}

// Is the node in weak-head normal form? (For our purposes, is it reduced to an integer?)
bool whnf(SLCNodePtr node)
{
    return node->tag == INT;
}

SLCNodePtr walk(SLCStack& stack, SLCNodePtr expr)
{
    stack.push(expr);
    while (expr->tag == APP)
    {
        // Unwind the spine as we try to find an actual function to apply.
        expr = std::get<APP>(expr->data).fun;
        stack.push(expr);
    }
    return expr;
}

void eval(SLCStack& stack, SLCNodePtr expr)
{
    auto node = expr;
    node = walk(stack, node);

    while (!whnf(expr))
    {
        switch (node->tag)
        {
            case LAM: {
                assert(stack.depth() - 1 >= 1);
                // Just before node will be the application: (node arg)
                SLCNodePtr& app = stack.at(stack.depth() - 2);
                SLCNodePtr& arg = std::get<APP>(app->data).arg;
                SLCNodePtr& body = std::get<LAM>(node->data).body;
                const std::string& param = std::get<LAM>(node->data).param;
                SLCNodePtr new_body = instantiate(body, param, arg);

                // Physically overwrite the node.
                app->data = new_body->data;
                app->tag = new_body->tag;
                stack.c.pop_back();
                stack.c.pop_back();
                node = walk(stack, app);
            } break;
            case BIF: {
                const std::string& name = std::get<std::string>(node->data);

                if (name == "IF")
                {
                    assert(stack.depth() - 1 >= 3);
                    SLCNodePtr& redex = stack.at(stack.depth() - 4);
                    SLCNodePtr& apptrue = std::get<APP>(redex->data).fun;
                    SLCNodePtr& appcond = std::get<APP>(apptrue->data).fun;
                    SLCNodePtr& cond = std::get<APP>(appcond->data).arg;
                    SLCNodePtr& extrue = std::get<APP>(apptrue->data).arg;
                    SLCNodePtr& exfalse = std::get<APP>(redex->data).arg;
                    stack.save();
                    eval(stack, cond);
                    stack.restore();

                    SLCNodePtr result;
                    if (std::get<INT>(cond->data) != 0)
                    {
                        result = extrue;
                    }
                    else
                    {
                        result = exfalse;
                    }

                    // Physically overwrite the node.
                    redex->data = result->data;
                    redex->tag = result->tag;
                }
                else
                {
                    assert(stack.depth() - 1 >= 2);
                    SLCNodePtr& redex = stack.at(stack.depth() - 3);
                    SLCNodePtr& inner = std::get<APP>(redex->data).fun;
                    SLCNodePtr& lhs = std::get<APP>(inner->data).arg;
                    SLCNodePtr& rhs = std::get<APP>(redex->data).arg;
                    stack.save();
                    eval(stack, lhs);
                    stack.restore();
                    stack.save();
                    eval(stack, rhs);
                    stack.restore();
                    SLCNodePtr result = builtin_map[name](lhs, rhs);
                    
                    // Physically overwrite the node.
                    redex->data = result->data;
                    redex->tag = result->tag;
                }
            } break;

            case INT:
            case VAR:
                // We are done.
                break;
            case APP:
                assert(0 && "Unreachable: handled by while loop.");
        }
    }
}

SLCNodePtr get_program();

int main()
{
    SLCStack stack {};

    SLCNodePtr expr = get_program();
    eval(stack, expr);
    printf("O0 Output: ");
    print(expr);
    printf("\n");

    return EXIT_SUCCESS;
}

// SLCNodePtr get_program()
// {
//     auto x = std::make_shared<SLCNode>(VAR, "x");
//     auto y = std::make_shared<SLCNode>(VAR, "y");
//     auto ADD = std::make_shared<SLCNode>(BIF, "ADD");
//     auto addX = std::make_shared<SLCNode>(APP, SLCNodeApp {ADD, x});
//     auto addXY = std::make_shared<SLCNode>(APP, SLCNodeApp {addX, y});
//     auto l1 = std::make_shared<SLCNode>(LAM, SLCNodeLam {"y", addXY});
//     auto l2 = std::make_shared<SLCNode>(LAM, SLCNodeLam {"x", l1});
//     auto a = std::make_shared<SLCNode>(INT, 1330);
//     auto b = std::make_shared<SLCNode>(INT, 7);
//     auto l2ApplyA = std::make_shared<SLCNode>(APP, SLCNodeApp {l2, a});
//     auto expr = std::make_shared<SLCNode>(APP, SLCNodeApp {l2ApplyA, b});
//     return expr;
// }
