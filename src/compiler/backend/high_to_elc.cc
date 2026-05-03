#include "high_to_elc.hh"

#include "../common.hh"

void TreeToELCVisitor::visit(TreeSeqNode *node)
{
    /*
      Program:
      Definition1
      ...
      DefinitionN
      -----------
      Main Expression
      
      |
      V

      letrec
        TD[[ Definition1 ]]
        ...
        TD[[ DefinitionN ]]
      in
        TE[[ Main Expression ]]
     */

    LCNodePtr main_expr = nullptr;

    std::vector<LCNodePtr> elc_children;
    for (auto& child : node->children) {
        v_is_toplevel.write(true);
        child->accept(this);
        LCNodePtr def = v_elc.read_asserted();
        LCDefNode *def_ptr = static_cast<LCDefNode *>(def.get());

        if (def_ptr->var == "main") {
            main_expr = def_ptr->body;
        } else {
            elc_children.push_back(def);
        }
    }

    if (!main_expr) {
        COMPILER_ERROR_TERM("Function binding 'main' not found.");
    }

    v_elc.write(std::make_shared<LCLetNode>(elc_children, main_expr, RECURSIVE));
}

void TreeToELCVisitor::visit(TreeParamsNode *node)
{
    /*
      By this point, the let body expression should be in v_elc

      We simply wrap the resulting node within a lambda abstraction
    */

    LCNodePtr body = v_elc.read_asserted();
    LCNodePtr lambda = body;

    for (auto it = node->children.rbegin(); it != node->children.rend(); ++it) {
        (*it)->accept(this);
        LCNodePtr lc = v_elc.read_asserted();
        auto constant = static_cast<LCConstantNode *>(lc.get());

        lambda = std::make_shared<LCLambdaNode>(constant->name, lambda);
    }

    v_elc.write(lambda);
}

void TreeToELCVisitor::visit(TreeListNode *node)
{
    (void)node;
    COMPILER_ERROR_TERM("TODO: High->ELC for TreeListNode");
}

void TreeToELCVisitor::visit(TreeBindingNode *node)
{
    /*
      let x: int := 5,
      let y: int := 6,
      x + y
     */
    
    /*
      TD[[ let v: T := E ]] == v = TE[[ E ]]
     */


    // We are no longer looking at the top level
    bool toplevel = v_is_toplevel.read_asserted();
    v_is_toplevel.write(false);
    
    // Look into the body
    node->body->accept(this);

    // Restore the toplevel assertion
    v_is_toplevel.write(toplevel);

    if (node->params != nullptr) {
        // If this is a function, wrap the body in lambdas
        node->params->accept(this);
    }

    LCNodePtr body = v_elc.read_asserted();
    LCNodePtr def = std::make_shared<LCDefNode>(node->id, body);

    if (v_is_toplevel.read_asserted()) {
        /*
          We should get a def, (v = B)
        */
        v_is_toplevel.write(true);

        if (node->next != nullptr) {
            INTERNAL_ERROR("A top-level let (`{}`) should not have a next node.", node->id);
        }

        v_elc.write(def);
    } else {
        /*
          We should get a full let expression (let v = B in E)
        */
        v_is_toplevel.write(false);
        
        if (node->next == nullptr) {
            INTERNAL_ERROR("A non-top-level let should have a next node, as there must be some final expression.");
        }

        node->next->accept(this);
        LCNodePtr in_expr = v_elc.read_asserted();

        // TODO: Don't blindly pick recursive by default
        LCNodePtr let = std::make_shared<LCLetNode>(std::vector<LCNodePtr> {def}, in_expr, RECURSIVE);
        v_elc.write(let);
    }
}

std::string token_to_builtin(TokenType token)
{
    switch (token) {
    case TokenType::Add: return "ADD";
    case TokenType::Sub: return "SUB";
    case TokenType::Mul: return "MUL";
    case TokenType::Div: return "DIV";
    case TokenType::Equ: return "EQU";
    default: COMPILER_ERROR_TERM("Unknown builtin");
    }
}

void TreeToELCVisitor::visit(TreeBinopNode *node)
{
    /*
      TE[[ E1 <infix> E2 ]] == TE[[ <infix> ]] TE[[ E1 ]] TE[[ E2 ]]
     */
    node->lhs->accept(this);
    LCNodePtr elc_lhs = v_elc.read_asserted();
    node->rhs->accept(this);
    LCNodePtr elc_rhs = v_elc.read_asserted();
    std::string builtin = token_to_builtin(node->op);

    LCNodePtr elc_fn = std::make_shared<LCConstantNode>(builtin);
    LCNodePtr curried = std::make_shared<LCApplyNode>(elc_fn, elc_lhs);
    v_elc.write(std::make_shared<LCApplyNode>(curried, elc_rhs));
}

void TreeToELCVisitor::visit(TreeApplyNode *node)
{
    /*
      TE[[ E1 E2 ]] == TE[[ E1 ]] TE[[ E2 ]]
     */
    node->func->accept(this);
    LCNodePtr elc_fun = v_elc.read_asserted();
    node->arg->accept(this);
    LCNodePtr elc_arg = v_elc.read_asserted();
    v_elc.write(std::make_shared<LCApplyNode>(elc_fun, elc_arg));
}

void TreeToELCVisitor::visit(TreeMatchNode *node)
{
    /*
      This one is harder, as we have a slight deviation from Miranda.

      Miranda:

      factorial 0 = 1
      factorial n = n * factorial (n - 1)

      My language:

      let factorial: int [x: int] :=
          match x with
              0 -> 1
              n -> n * factorial (n - 1)
          end
      end
     */
    
    (void)node;
    COMPILER_ERROR_TERM("TODO: High->ELC for TreeMatchNode");
}

void TreeToELCVisitor::visit(TreeMatchArmNode *node)
{
    (void)node;
    COMPILER_ERROR_TERM("TODO: High->ELC for TreeMatchArmNode");
}

void TreeToELCVisitor::visit(TreeIfNode *node)
{
    node->cond->accept(this);
    LCNodePtr elc_cond = v_elc.read_asserted();
    node->extrue->accept(this);
    LCNodePtr elc_extrue = v_elc.read_asserted();
    node->exfalse->accept(this);
    LCNodePtr elc_exfalse = v_elc.read_asserted();

    LCNodePtr elc_fn = std::make_shared<LCConstantNode>("IF");
    LCNodePtr elc_ifcond = std::make_shared<LCApplyNode>(elc_fn, elc_cond);
    LCNodePtr elc_ifcondt = std::make_shared<LCApplyNode>(elc_ifcond, elc_extrue);
    LCNodePtr elc_ifcondtf = std::make_shared<LCApplyNode>(elc_ifcondt, elc_exfalse);
    v_elc.write(elc_ifcondtf);
}

void TreeToELCVisitor::visit(TreeIdentNode *node)
{
    /*
      TE[[ var/const ]] == var/const
     */
    v_elc.write(std::make_shared<LCConstantNode>(node->name));
}

void TreeToELCVisitor::visit(TreeIntegerNode *node)
{
    /*
      TE[[ 5 ]] == 5
     */
    v_elc.write(std::make_shared<LCIntNode>(node->value));
}

void TreeToELCVisitor::visit(TreeStringNode *node)
{
    (void)node;
    COMPILER_ERROR_TERM("TODO: High->ELC for TreeStringNode");
}
