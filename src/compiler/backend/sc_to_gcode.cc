#include "./sc_to_gcode.hh"

#include "../common.hh"

void compile_builtins(SCToGCodeCompiler *compiler);

void SCToGCodeCompiler::compile(LiftedProgram& lp)
{
    output.push_back(std::make_unique<GInstrBegin>());

    // Assume PROG supercombinator is the only funcall of the lifted program.
    auto node = dynamic_cast<LCConstantNode *>(lp.lifted_prog.get());
    assert(node != nullptr);
    output.push_back(std::make_unique<GInstrPushGlobal>(node->name));

    output.push_back(std::make_unique<GInstrEval>());
    output.push_back(std::make_unique<GInstrPrint>());

    output.push_back(std::make_unique<GInstrEnd>());

    for (auto& [sc_name, lcbody] : lp.sc_visitor.supercombinators) {
        compile_supercombinator(sc_name, lcbody);
    }

    // Gonna see if we can avoid this completely.
    // compile_builtins(this);
}

void compile_builtins(SCToGCodeCompiler *compiler)
{
    // TODO: make sure this compiles
    compiler->output.push_back(std::make_unique<GInstrGlobStart>("ADD", 2));
    compiler->output.push_back(std::make_unique<GInstrPush>(1));
    compiler->output.push_back(std::make_unique<GInstrEval>());
    compiler->output.push_back(std::make_unique<GInstrPush>(1));
    compiler->output.push_back(std::make_unique<GInstrEval>());
    compiler->output.push_back(std::make_unique<GInstrBinOp>(GBinop::Add));
    compiler->output.push_back(std::make_unique<GInstrUpdate>(3));
    compiler->output.push_back(std::make_unique<GInstrPop>(2));
    compiler->output.push_back(std::make_unique<GInstrUnwind>());
    compiler->output.push_back(std::make_unique<GInstrGlobEnd>("ADD", 2));

    compiler->output.push_back(std::make_unique<GInstrGlobStart>("SUB", 2));
    compiler->output.push_back(std::make_unique<GInstrPush>(1));
    compiler->output.push_back(std::make_unique<GInstrEval>());
    compiler->output.push_back(std::make_unique<GInstrPush>(1));
    compiler->output.push_back(std::make_unique<GInstrEval>());
    compiler->output.push_back(std::make_unique<GInstrBinOp>(GBinop::Sub));
    compiler->output.push_back(std::make_unique<GInstrUpdate>(3));
    compiler->output.push_back(std::make_unique<GInstrPop>(2));
    compiler->output.push_back(std::make_unique<GInstrUnwind>());
    compiler->output.push_back(std::make_unique<GInstrGlobEnd>("SUB", 2));

    compiler->output.push_back(std::make_unique<GInstrGlobStart>("MUL", 2));
    compiler->output.push_back(std::make_unique<GInstrPush>(1));
    compiler->output.push_back(std::make_unique<GInstrEval>());
    compiler->output.push_back(std::make_unique<GInstrPush>(1));
    compiler->output.push_back(std::make_unique<GInstrEval>());
    compiler->output.push_back(std::make_unique<GInstrBinOp>(GBinop::Mul));
    compiler->output.push_back(std::make_unique<GInstrUpdate>(3));
    compiler->output.push_back(std::make_unique<GInstrPop>(2));
    compiler->output.push_back(std::make_unique<GInstrUnwind>());
    compiler->output.push_back(std::make_unique<GInstrGlobEnd>("MUL", 2));

    compiler->output.push_back(std::make_unique<GInstrGlobStart>("DIV", 2));
    compiler->output.push_back(std::make_unique<GInstrPush>(1));
    compiler->output.push_back(std::make_unique<GInstrEval>());
    compiler->output.push_back(std::make_unique<GInstrPush>(1));
    compiler->output.push_back(std::make_unique<GInstrEval>());
    compiler->output.push_back(std::make_unique<GInstrBinOp>(GBinop::Div));
    compiler->output.push_back(std::make_unique<GInstrUpdate>(3));
    compiler->output.push_back(std::make_unique<GInstrPop>(2));
    compiler->output.push_back(std::make_unique<GInstrUnwind>());
    compiler->output.push_back(std::make_unique<GInstrGlobEnd>("DIV", 2));
}

int count_arguments(LCNodePtr lcbody)
{
    int count = 0;
    LCNodePtr node = lcbody;
    for (auto lambda_ptr = dynamic_cast<LCLambdaNode *>(node.get());
         lambda_ptr != nullptr;
         lambda_ptr = dynamic_cast<LCLambdaNode *>(node.get()))
    {
        ++count;
        node = lambda_ptr->body;
    }
    return count;
}

LCNodePtr setup_environment(GCodeEnv& env, LCNodePtr lcbody, int n)
{
    int count = n;
    LCNodePtr node = lcbody;
    for (auto lambda_ptr = dynamic_cast<LCLambdaNode *>(node.get());
         lambda_ptr != nullptr;
         lambda_ptr = dynamic_cast<LCLambdaNode *>(node.get()))
    {
        env.offset_map[lambda_ptr->param] = count--;
        node = lambda_ptr->body;
    }
    return node;
}

/*
p takes a "name" and returns the offset of the argument from the base of the current context.
  Bottom element of the current context has a value of 0.
  Pointer to current redex has an offset of 0, and the last argument has an offset of 1.
d is the depth of the current context, minus 1.

The offset of a variable from [the top of the stack] is (d - p(x)), assuming the top of the stack
has the offset of 0.
*/

/*
  F[[ $F x1 x2 = E ]] scheme

  = GLOBSTART f, n; R [[ E ]] [x1=n, x2=n-1, ..., xn=1] n
*/
void SCToGCodeCompiler::compile_supercombinator(const std::string& sc_name, LCNodePtr lcbody)
{
    int n = count_arguments(lcbody);
    output.push_back(std::make_unique<GInstrGlobStart>(sc_name, n));

    GCodeEnv env {n};
    LCNodePtr inner_body = setup_environment(env, lcbody, n);
    // fmt::println("{} of {} args.", sc_name, n);
    // for (auto& [var, off] : env.offset_map) {
    //     fmt::println("  {} => {}", var, off);
    // }
    
    compile_supercombinator_body(env, inner_body);

    output.push_back(std::make_unique<GInstrGlobEnd>(sc_name, n));
}

/*
  R[[ E ]] scheme
*/
void SCToGCodeCompiler::compile_supercombinator_body(GCodeEnv& env, LCNodePtr body)
{
    // Performs badly on a body of 1 variable, revise R[[]] with Chapter 20
    compile_construct_instance(&output, &env, body);

    output.push_back(std::make_unique<GInstrUpdate>(env.depth + 1));
    output.push_back(std::make_unique<GInstrPop>(env.depth));
    output.push_back(std::make_unique<GInstrUnwind>());
}

/*
  C[[ E ]] scheme
*/
void compile_construct_instance(std::vector<GInstrPtr> *output, const GCodeEnv *env, LCNodePtr body)
{
    LCConstructInstanceVisitor visitor(output, env);
    body->accept(&visitor);
}

/*
  C[[ E ]] as a visitor.
*/

void LCConstructInstanceVisitor::visit(LCApplyNode *node)
{
    // C[[ E1 E2 ]] = C[[ E2 ]] p d; C[[ E1 ]] p (d+1); MKAP
    compile_construct_instance(output, env, node->arg);

    // TODO: I have zero clue if I need to save this or not, book doesn't say.
    GCodeEnv new_env = *env;
    new_env.depth = env->depth + 1;
    compile_construct_instance(output, &new_env, node->fun);

    output->push_back(std::make_unique<GInstrMkApp>());
}

void LCConstructInstanceVisitor::visit(LCLambdaNode *node)
{
    /*
      Should never be called due to lambda lifting.
      The supercombinator bodies have lambdas to represent the parameters of the
      supercombinator, but they are peeled away before C[[]] is called.
    */
    UNUSED(node);
    INTERNAL_ERROR("TODO: LCLambdaNode ");
}

void LCConstructInstanceVisitor::visit(LCDefNode *node)
{
    UNUSED(node);
    INTERNAL_ERROR("TODO: LCDefNode ");
}

void LCConstructInstanceVisitor::visit(LCLetNode *node)
{
    // TODO: absolutely bad code, but for the purposes of speeding development...
    // We assume everything is a letrec even if it doesn't have to be.
    // To prevent this hack, I need to implement dependency analysis for lets (pg 118)
    node->recursive = false;
    if (node->recursive) {
        // Letrec
        INTERNAL_ERROR("TODO: C[[ letrec ]]");
        // BOOKMARK: pg 308
    } else {
        // Let
        //
        // C[[ let x = Ex in Eb ]] p d
        //   = C[[ Ex ]] p d; C[[ Eb ]] p[x=d+1] (d+1); SLIDE 1

        // UNSAFE
        // Paper only supports 1 definition inside a Let for now.
        assert(node->definitions.size() == 1);
        auto def_ptr = static_cast<LCDefNode *>(node->definitions[0].get());
        
        compile_construct_instance(output, env, def_ptr->body);

        GCodeEnv new_env = *env;
        new_env.offset_map[def_ptr->var] = env->depth + 1;
        new_env.depth = env->depth + 1;
        compile_construct_instance(output, &new_env, node->expr);

        output->push_back(std::make_unique<GInstrSlide>(1));
    }
}

void LCConstructInstanceVisitor::visit(LCCaseNode *node)
{
    UNUSED(node);
    INTERNAL_ERROR("TODO: LCCaseNode ");
}

void LCConstructInstanceVisitor::visit(LCCaseArmNode *node)
{
    UNUSED(node);
    INTERNAL_ERROR("TODO: LCCaseArmNode ");
}

void LCConstructInstanceVisitor::visit(LCIntNode *node)
{
    output->push_back(std::make_unique<GInstrPushInt>(node->value));
}

void LCConstructInstanceVisitor::visit(LCBoolNode *node)
{
    // Unsure if this is how I should do this
    output->push_back(std::make_unique<GInstrPushInt>((int)node->value));
}

void LCConstructInstanceVisitor::visit(LCConstantNode *node)
{
    // Note: I will need to discern between both variables and supercombinators/built-in functions
    auto it = env->offset_map.find(node->name);
    if (it != env->offset_map.end()) {
        // Found it, so it's a variable
        // C[[ x ]] = PUSH (d - p(x))
        output->push_back(std::make_unique<GInstrPush>(env->depth - it->second));
    } else {
        // Not found, assume it is a supercombinator or built-in function
        // C[[ f ]] = PUSHGLOBAL f
        output->push_back(std::make_unique<GInstrPushGlobal>(node->name));
    }
}

void LCConstructInstanceVisitor::visit(LCDummyNode *node)
{
    UNUSED(node);
    INTERNAL_ERROR("TODO: LCDummyNode ");
}
