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
        child->accept(this);
        auto [var, elc_child] = v_elc_let.read_asserted();

        if (var == "main") {
            main_expr = elc_child;
        } else {
            elc_children.push_back(std::make_shared<LCDefNode>(var, elc_child));
        }
    }

    if (!main_expr) {
        COMPILER_ERROR_TERM("Function binding 'main' not found.");
    }

    v_elc.write(std::make_shared<LCLetNode>(elc_children, main_expr, RECURSIVE));
}

void TreeToELCVisitor::visit(TreeParamsNode *node)
{
    (void)node;
    COMPILER_ERROR_TERM("TODO: High->ELC for TreeParamsNode");
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
    if (node->params != nullptr) {
        // TODO: simple function definition translation
        // node->params->accept(this);
    }

    node->body->accept(this);
    LCNodePtr body = v_elc.read_asserted();

    LCNodePtr def = std::make_shared<LCDefNode>(node->id, body);

    if (node->next != nullptr) {
        node->next->accept(this);
        LCNodePtr in_expr = v_elc.read_asserted();

        // TODO: recursive by default
        LCNodePtr let = std::make_shared<LCLetNode>(std::vector<LCNodePtr> {def}, in_expr, RECURSIVE);
        v_elc.write(let);
        v_elc_let.write({node->id, let});
    } else {
        // COMPILER_ERROR_TERM("No next node after a let binding, this might trigger for globals but it should be disallowed");
        LCNodePtr let = std::make_shared<LCLetNode>(std::vector<LCNodePtr> {def}, nullptr, RECURSIVE);
        v_elc.write(let);
        v_elc_let.write({node->id, let});
    }
}

std::string token_to_builtin(TokenType token)
{
    switch (token) {
    case TokenType::Add: return "ADD";
    case TokenType::Sub: return "SUB";
    case TokenType::Mul: return "MUL";
    case TokenType::Div: return "DIV";
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
