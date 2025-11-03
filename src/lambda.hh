#ifndef LAMBDA_HH_
#define LAMBDA_HH_

#include <memory>
#include <string>

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

/*
  Potential Functions:
  Free(string, LCNode) -> returns free occurances of a variable in a node
  
 */

struct LCNode;
using LCNodePtr = std::shared_ptr<LCNode>;

#define RECURSIVE true
#define NONRECURSIVE false

struct LCApplyNode;
struct LCLambdaNode;
struct LCDefNode;
struct LCLetNode;
struct LCIntNode;
struct LCBoolNode;
struct LCConstantNode;

struct ILCVisitor
{
    virtual void visit(LCApplyNode *node) = 0;
    virtual void visit(LCLambdaNode *node) = 0;
    virtual void visit(LCDefNode *node) = 0;
    virtual void visit(LCLetNode *node) = 0;
    virtual void visit(LCIntNode *node) = 0;
    virtual void visit(LCBoolNode *node) = 0;
    virtual void visit(LCConstantNode *node) = 0;
};

// Lambda Calculus Node
struct LCNode
{
    virtual ~LCNode()
    {}

    virtual void accept(ILCVisitor *visitor) = 0;
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
        : definitions(defs), expr(expr), recursive(recursive)
    {}

    virtual void accept(ILCVisitor *visitor)
    {
        visitor->visit(this);
    }
    
    std::vector<LCNodePtr> definitions;
    LCNodePtr expr;
    bool recursive;
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
    
    std::string name;
};

#endif // LAMBDA_HH_
