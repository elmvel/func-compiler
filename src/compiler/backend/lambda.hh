#ifndef LAMBDA_HH_
#define LAMBDA_HH_

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <fmt/core.h>

#include "../common.hh"

/*
  Syntax referenced from "The Implementation of Functional Programming Languages"
  by Simon Peyton Jones
 */

/*
  Built-ins to account for:
  +, -, *, /
  AND, OR, NOT,
  TRUE, FALSE,
  'a', 'b', ...,

  Built-ins with reduction rules:
  IF TRUE  Et Ef -> Et
  IF FALSE Et Ef -> Ef

  Data constructors with reduction rules:
  HEAD (CONS a b) -> a
  TAIL (CONS a b) -> b
  or NIL for empty list
 */

/*
  Implementation Concerns:
  Graph-based, potential for tree-based deallocation (delete root -> delete leaves)
  to be incorrect.

  CONS, HEAD, TAIL can be implemented as lambda abstractions, but not particularly
  efficient to do so -> built-in for efficiency.
 */

struct LCNode;
using LCNodePtr = std::shared_ptr<LCNode>;
using LevelNo = int;
using LCSupercombinatorMap = std::unordered_map<std::string, LCNodePtr>;

#define RECURSIVE true
#define NONRECURSIVE false

/*
  Book-keeping for free variables.
*/
struct FreeVarsEnv
{
    FreeVarsEnv()
        : free_vars(), level(0)
    {}
    
    std::unordered_map<std::string, LevelNo> free_vars;
    std::unordered_set<std::string> bound_vars = {
        "ADD",
        "SUB",
        "MUL",
        "DIV",
        "EQU",
        "IF",
    };
    LevelNo level;
};

struct LCApplyNode;
struct LCLambdaNode;
struct LCDefNode;
struct LCLetNode;
struct LCCaseNode;
struct LCCaseArmNode;
struct LCIntNode;
struct LCBoolNode;
struct LCConstantNode;
struct LCDummyNode;

struct ILCVisitor
{
    virtual void visit(LCApplyNode *node) = 0;
    virtual void visit(LCLambdaNode *node) = 0;
    virtual void visit(LCDefNode *node) = 0;
    virtual void visit(LCLetNode *node) = 0;
    virtual void visit(LCCaseNode *node) = 0;
    virtual void visit(LCCaseArmNode *node) = 0;
    virtual void visit(LCIntNode *node) = 0;
    virtual void visit(LCBoolNode *node) = 0;
    virtual void visit(LCConstantNode *node) = 0;
    virtual void visit(LCDummyNode *node) = 0;
};

struct ILCTransformVisitor
{
    virtual LCNodePtr visit(LCApplyNode *node) = 0;
    virtual LCNodePtr visit(LCLambdaNode *node) = 0;
    virtual LCNodePtr visit(LCDefNode *node) = 0;
    virtual LCNodePtr visit(LCLetNode *node) = 0;
    virtual LCNodePtr visit(LCCaseNode *node) = 0;
    virtual LCNodePtr visit(LCCaseArmNode *node) = 0;
    virtual LCNodePtr visit(LCIntNode *node) = 0;
    virtual LCNodePtr visit(LCBoolNode *node) = 0;
    virtual LCNodePtr visit(LCConstantNode *node) = 0;
    virtual LCNodePtr visit(LCDummyNode *node) = 0;
};

// Lambda Calculus Node
struct LCNode
{
    const LevelNo LEVELNO_DEFAULT = 0;
    
    virtual ~LCNode()
    {}

    virtual void accept(ILCVisitor *visitor) = 0;
    virtual LCNodePtr accept(ILCTransformVisitor *visitor) = 0;

    /*
      This one is sneaky. To avoid code repetition, and since we will
      surely try to find the free variables at least once, we will also
      compute the let(rec) lexical levels here.
    */
    virtual void find_free_vars(FreeVarsEnv& env) = 0;

    virtual int count_inner_lambdas() = 0;
    virtual void sc_rewrite(const std::string& from, const std::string& to) = 0;
};

/*
  Lambda Calculus Representation:
  (f x)
 */
struct LCApplyNode : LCNode
{
    LCApplyNode(LCNodePtr fun, LCNodePtr arg)
        : fun(fun), arg(arg)
    {}

    virtual ~LCApplyNode()
    {
    }
    
    virtual void accept(ILCVisitor *visitor)
    {
        visitor->visit(this);
    }

    virtual LCNodePtr accept(ILCTransformVisitor *visitor)
    {
        return visitor->visit(this);
    }

    virtual void find_free_vars(FreeVarsEnv& env)
    {
        fun->find_free_vars(env);
        arg->find_free_vars(env);
    }

    virtual int count_inner_lambdas()
    {
        return fun->count_inner_lambdas() + arg->count_inner_lambdas();
    }

    virtual void sc_rewrite(const std::string& from, const std::string& to)
    {
        fun->sc_rewrite(from, to);
        arg->sc_rewrite(from, to);
    }

    LCNodePtr fun;
    LCNodePtr arg;
};

/*
  Lambda Calculus Representation:
  (\ x . + x 1)
 */
struct LCLambdaNode : LCNode
{
    LCLambdaNode(const std::string& param, LCNodePtr body)
        : param(param), body(body)
    {}

    virtual ~LCLambdaNode()
    {
    }

    virtual void accept(ILCVisitor *visitor)
    {
        visitor->visit(this);
    }

    virtual LCNodePtr accept(ILCTransformVisitor *visitor)
    {
        return visitor->visit(this);
    }

    virtual void find_free_vars(FreeVarsEnv& env)
    {
        // Since this is a lambda, we will keep track of the lexical number.
        env.level += 1;
        
        env.bound_vars.insert(param);
        body->find_free_vars(env);
        env.bound_vars.erase(param);

        // Done with the lambda, decrease the lexical number.
        env.level -= 1;
    }

    virtual int count_inner_lambdas()
    {
        return (1) + body->count_inner_lambdas();
    }

    virtual void sc_rewrite(const std::string& from, const std::string& to)
    {
        body->sc_rewrite(from, to);
    }
    
    std::string param;
    LCNodePtr body;
};

/*
  The 'v = B' in (let v = B in E)
 */
struct LCDefNode : LCNode
{
    LCDefNode(const std::string& var, LCNodePtr body)
        : var(var), body(body)
    {}

    virtual void accept(ILCVisitor *visitor)
    {
        visitor->visit(this);
    }

    virtual LCNodePtr accept(ILCTransformVisitor *visitor)
    {
        return visitor->visit(this);
    }

    virtual void find_free_vars(FreeVarsEnv& env)
    {
        UNUSED(env);
        INTERNAL_ERROR("We should not be calling find_free_vars on an LCDefNode.");
    }

    virtual int count_inner_lambdas()
    {
        return body->count_inner_lambdas();
    }

    virtual void sc_rewrite(const std::string& from, const std::string& to)
    {
        body->sc_rewrite(from, to);
    }
    
    std::string var;
    LCNodePtr body;
};

/*
  Lambda Calculus Representation:
  (let v = B in E)
  (let x = 3 in (* x x))
 */
struct LCLetNode : LCNode
{
    LCLetNode(const std::vector<LCNodePtr>& defs, LCNodePtr expr, bool recursive)
        : definitions(defs), expr(expr), recursive(recursive), level(-1)
    {}

    virtual void accept(ILCVisitor *visitor)
    {
        visitor->visit(this);
    }

    virtual LCNodePtr accept(ILCTransformVisitor *visitor)
    {
        return visitor->visit(this);
    }

    virtual void find_free_vars(FreeVarsEnv& env)
    {
        // Before starting, record the level of the let(rec)
        level = env.level;
        
        std::unordered_set<std::string> let_defined;

        // First, find the free variables in the definition bodies.
        for (auto& node : definitions) {
            LCDefNode *def = static_cast<LCDefNode *>(node.get());
            
            def->body->find_free_vars(env);
            let_defined.insert(def->var);
        }

        // Then, add the defined variables to the environment.
        for (auto& var : let_defined) {
            env.bound_vars.insert(var);
        }

        // Find the free variable within the let body.
        expr->find_free_vars(env);

        // Finally, remove all variables that were defined by the let.
        for (auto& var : let_defined) {
            env.bound_vars.erase(var);
        }
    }

    virtual int count_inner_lambdas()
    {
        int count = 0;
        for (auto& node : definitions) {
            count += node->count_inner_lambdas();
        }
        return count + expr->count_inner_lambdas();
    }

    virtual void sc_rewrite(const std::string& from, const std::string& to)
    {
        for (auto& node : definitions) {
            node->sc_rewrite(from, to);
        }
        expr->sc_rewrite(from, to);
    }
    
    std::vector<LCNodePtr> definitions;
    LCNodePtr expr;
    bool recursive;
    int level;
};

/*
  Lambda Calculus Representation:

  Nonsensical example
  (case t of
      Leaf n       => Leaf n
      Branch t1 t2 => Leaf 0
  )
 */
struct LCCaseNode : LCNode
{
    LCCaseNode(LCNodePtr scrutinee, const std::vector<LCNodePtr>& arms)
        : scrutinee(scrutinee), arms(arms)
    {}

    virtual void accept(ILCVisitor *visitor)
    {
        visitor->visit(this);
    }

    virtual LCNodePtr accept(ILCTransformVisitor *visitor)
    {
        return visitor->visit(this);
    }

    virtual void find_free_vars(FreeVarsEnv& env)
    {
        for (auto& arm : arms) {
            arm->find_free_vars(env);
        }
    }

    virtual int count_inner_lambdas()
    {
        int count = 0;
        for (auto& arm : arms) {
            count += arm->count_inner_lambdas();
        }
        return count;
    }

    virtual void sc_rewrite(const std::string& from, const std::string& to)
    {
        scrutinee->sc_rewrite(from, to);
        for (auto& arm : arms) {
            arm->sc_rewrite(from, to);
        }
    }

    LCNodePtr scrutinee;
    std::vector<LCNodePtr> arms;
};

/*
  Lambda Calculus Representation:

  Nonsensical example
  case t of
      (Leaf n      => Leaf n) <---
      Branch t1 t2 => Leaf 0
 */
struct LCCaseArmNode : LCNode
{
    LCCaseArmNode(LCNodePtr pattern, LCNodePtr body)
        : pattern(pattern), body(body)
    {}

    virtual void accept(ILCVisitor *visitor)
    {
        visitor->visit(this);
    }

    virtual LCNodePtr accept(ILCTransformVisitor *visitor)
    {
        return visitor->visit(this);
    }

    virtual void find_free_vars(FreeVarsEnv& env)
    {
        UNUSED(env);
        INTERNAL_ERROR("TODO: support match arms when needed");
    }

    virtual int count_inner_lambdas()
    {
        INTERNAL_ERROR("TODO: support match arms when needed");
    }

    virtual void sc_rewrite(const std::string& from, const std::string& to)
    {
        UNUSED(from);
        UNUSED(to);
        INTERNAL_ERROR("TODO: support match arms when needed");
    }

    LCNodePtr pattern;
    LCNodePtr body;
};

/*
  Lamda Calculus Representation:
  (5)
 */
struct LCIntNode : LCNode
{
    LCIntNode(int value)
        : value(value)
    {}

    virtual void accept(ILCVisitor *visitor)
    {
        visitor->visit(this);
    }

    virtual LCNodePtr accept(ILCTransformVisitor *visitor)
    {
        return visitor->visit(this);
    }

    virtual void find_free_vars(FreeVarsEnv& env)
    {
        UNUSED(env);
    }

    virtual int count_inner_lambdas()
    {
        return 0;
    }

    virtual void sc_rewrite(const std::string& from, const std::string& to)
    {
        UNUSED(from);
        UNUSED(to);
    }
    
    int value;
};

/*
  Lamda Calculus Representation:
  (TRUE) or (FALSE)
 */
struct LCBoolNode : LCNode
{
    LCBoolNode(bool value)
        : value(value)
    {}

    virtual void accept(ILCVisitor *visitor)
    {
        visitor->visit(this);
    }

    virtual LCNodePtr accept(ILCTransformVisitor *visitor)
    {
        return visitor->visit(this);
    }

    virtual void find_free_vars(FreeVarsEnv& env)
    {
        UNUSED(env);
    }

    virtual int count_inner_lambdas()
    {
        return 0;
    }

    virtual void sc_rewrite(const std::string& from, const std::string& to)
    {
        UNUSED(from);
        UNUSED(to);
    }
    
    bool value;
};

/*
  Lamda Calculus Representation:
  (TRUE) or (FALSE)
 */
struct LCConstantNode : LCNode
{
    LCConstantNode(const std::string& name)
        : name(name)
    {}

    virtual void accept(ILCVisitor *visitor)
    {
        visitor->visit(this);
    }

    virtual LCNodePtr accept(ILCTransformVisitor *visitor)
    {
        return visitor->visit(this);
    }

    virtual void find_free_vars(FreeVarsEnv& env)
    {
        if (env.bound_vars.find(name) == env.bound_vars.end()) {
            // 'name' was not bound
            env.free_vars[name] = env.level;
        }
    }

    virtual int count_inner_lambdas()
    {
        return 0;
    }

    virtual void sc_rewrite(const std::string& from, const std::string& to)
    {
        if (name == from) {
            name = to;
        }
    }
    
    std::string name;
};

// TODO: why do I have this again?
struct LCDummyNode : LCNode
{
    LCDummyNode()
    {}

    virtual void accept(ILCVisitor *visitor)
    {
        visitor->visit(this);
    }

    virtual LCNodePtr accept(ILCTransformVisitor *visitor)
    {
        return visitor->visit(this);
    }

    virtual void find_free_vars(FreeVarsEnv& env)
    {
        UNUSED(env);
        INTERNAL_ERROR("No free variable in a dummy node.");
    }

    virtual int count_inner_lambdas()
    {
        return 0;
    }

    virtual void sc_rewrite(const std::string& from, const std::string& to)
    {
        UNUSED(from);
        UNUSED(to);
    }
};

#endif // LAMBDA_HH_
