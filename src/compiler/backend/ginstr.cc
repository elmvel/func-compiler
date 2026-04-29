#include <fmt/core.h>
#include <fmt/std.h>

#include "./ginstr.hh"
#include "../common.hh"

std::string_view binop_to_name(GBinop binop)
{
    switch (binop) {
        case GBinop::Add: return "Add";
        case GBinop::Sub: return "Sub";
        case GBinop::Mul: return "Mul";
        case GBinop::Div: return "Div";
        default: INTERNAL_ERROR("Unreachable binop name.");
    }
}

void print_indentation(int level)
{
    for (int i = 0; i < level; ++i)
        fmt::print("  ");
}

void GInstrPushInt::dump(int level)
{
    print_indentation(level);
    fmt::println("PushInt({})", value);
}

void GInstrPushGlobal::dump(int level)
{
    print_indentation(level);
    fmt::println("PushGlobal({})", name);
}

void GInstrPush::dump(int level)
{
    print_indentation(level);
    fmt::println("Push({})", offset);
}

void GInstrPop::dump(int level)
{
    print_indentation(level);
    fmt::println("Pop({})", n);
}

void GInstrMkApp::dump(int level)
{
    print_indentation(level);
    fmt::println("MkApp()");
}

void GInstrUnwind::dump(int level)
{
    print_indentation(level);
    fmt::println("Unwind()");
}

void GInstrUpdate::dump(int level)
{
    print_indentation(level);
    fmt::println("Update({})", offset);
}

void GInstrPack::dump(int level)
{
    print_indentation(level);
    fmt::println("Pack({}, {})", tag, n);
}

void GInstrSplit::dump(int level)
{
    print_indentation(level);
    fmt::println("Split()");
}

void GInstrJump::dump(int level)
{
    print_indentation(level);
    // TODO: implement debug printing for GInstrJump
    fmt::println("Jump(...<printing unimplemented>...)");
}

void GInstrSlide::dump(int level)
{
    print_indentation(level);
    fmt::println("Slide({})", n);
}

void GInstrBinOp::dump(int level)
{
    print_indentation(level);
    fmt::print("BinOp(");
    fmt::print("{}", binop_to_name(this->binop));
    fmt::println(")");
}

void GInstrEval::dump(int level)
{
    print_indentation(level);
    fmt::println("Eval()");
}

void GInstrAlloc::dump(int level)
{
    print_indentation(level);
    fmt::println("Alloc({})", n);
}

void GInstrBegin::dump(int level)
{
    print_indentation(level);
    fmt::println("Begin()");
}

void GInstrEnd::dump(int level)
{
    print_indentation(level);
    fmt::println("End()");
}

void GInstrPrint::dump(int level)
{
    print_indentation(level);
    fmt::println("Print()");
}

void GInstrGlobStart::dump(int level)
{
    print_indentation(level);
    fmt::println("GlobStart({}, {})", name, n);
}

/*
  Compilation
*/
void GInstrPushInt::compile(std::string& output)
{
    output.append("    // GInstrPushInt\n");
    output.append("    {\n");
    output.append(fmt::format("    node_t *node = gm_alloc_node_number({});\n", this->value));
    output.append(fmt::format("    gm_stack_push(stack, node);\n"));
    output.append("    }\n");
}

void GInstrPushGlobal::compile(std::string& output)
{
    output.append("    // GInstrPushGlobal\n");
    output.append("    {\n");
    output.append(fmt::format("    node_t *node = gm_alloc_node_func({});\n", this->name));
    output.append("    gm_stack_push(stack, node);\n");
    output.append("    }\n");
}

void GInstrPush::compile(std::string& output)
{
    // <n0:n1:...:nk:S, G, PUSH k:C, D>
    // => <nk:n0:n1:...:nk:S, G, C, D>
    output.append("    // GInstrPush\n");
    output.append("    {\n");
    output.append(fmt::format("    node_t *node = *gm_stack_peek(stack, {});\n", this->offset));
    output.append("    gm_stack_push(stack, node);\n");
    output.append("    }\n");
}

void GInstrPop::compile(std::string& output)
{
    output.append("    // GInstrPop\n");
    output.append(fmt::format("    for (size_t i = 0; i < {}; ++i)\n", n));
    output.append("    {\n");
    output.append("    gm_stack_pop(stack);\n");
    output.append("    }\n");
    // UNUSED(output);
    // INTERNAL_ERROR("TODO: GInstrPop");
}

void GInstrMkApp::compile(std::string& output)
{
    output.append("    // GInstrMkApp\n");
    output.append("    {\n");
    output.append("    if (stack->count < 2) RUNTIME_ERROR(\"Not enough on the stack for MkApp.\");\n");
    output.append("    node_t *n1 = gm_stack_pop(stack);\n");
    output.append("    node_t *n2 = gm_stack_pop(stack);\n");
    output.append("    node_t *n  = gm_alloc_node_app(n1, n2);\n");
    output.append("    gm_stack_push(stack, n);\n");
    output.append("    }\n");
    // UNUSED(output);
    // INTERNAL_ERROR("TODO: GInstrMkApp");
}

void GInstrUnwind::compile(std::string& output)
{
    output.append("    // GInstrUnwind\n");
    /*
      for (;;) {
          node_t *node = *gm_stack_peek(stack, 0);
          if (node->tag == NInteger) {
              // Restore saved stack and code from the dump, and put the node on top of the restored stack.
              break;
          } else if (node->tag == NApply) {
              // node->data.app.fun gets pushed onto the stack, repeat UNWIND
              gm_stack_push(stack, node->data.app.fun);
              // Somehow repeat UNWIND, maybe keep all of this in a loop?
              // ^- Thus, do not break here.
          } else if (node->tag == NFunc) {
              // Also check if there are enough arguments on the stack

              // If there are enough arguments...
              // Rearrange the stack as per Section 18.5.1 (SPJ),
              //   and begin executing the code for the function

              // If there are NOT enough arguments...
              // Restore saved stack and code from the dump, and put the node on top of the restored stack.
              break;
          }
      }
    */
    // We're just going to omit this to see what else I can implement before coming back here.
    // INTERNAL_ERROR("TODO: GInstrUnwind");
    fmt::println("Warning: UNWIND not implemented yet.");
}

void GInstrUpdate::compile(std::string& output)
{
    // n0 n1 n2 n3 n4 n5 .. nk
    //this->offset;
    output.append("    // GInstrUpdate\n");
    output.append("    {\n");
    output.append(fmt::format("    if (stack->count <= {}) RUNTIME_ERROR(\"Not enough stack space for Update {}\");\n", offset, offset));
    output.append("    node_t *top = *gm_stack_peek(stack, 0);\n");
    output.append("    node_t *ind = gm_alloc_node_ind(top);");
    output.append(fmt::format("    node_t **to_replace = gm_stack_peek(stack, {});\n", offset));
    output.append("    *to_replace = ind;\n");
    output.append("    }\n");
    // UNUSED(output);
    // INTERNAL_ERROR("TODO: GInstrUpdate");
}

void GInstrPack::compile(std::string& output)
{
    output.append("    // GInstrPack\n");
    UNUSED(output);
    INTERNAL_ERROR("TODO: GInstrPack");
}

void GInstrSplit::compile(std::string& output)
{
    output.append("    // GInstrSplit\n");
    UNUSED(output);
    INTERNAL_ERROR("TODO: GInstrSplit");
}

void GInstrJump::compile(std::string& output)
{
    output.append("    // GInstrJump\n");
    UNUSED(output);
    INTERNAL_ERROR("TODO: GInstrJump");
}

void GInstrSlide::compile(std::string& output)
{
    // for k = 3:
    // n0 n1 n2 n3 n4 n5 n6
    // =>
    //          n0 n4 n5 n6

    // node n = pop()
    // for k-1 times: pop()
    // set the top node to n

    output.append("    // GInstrSlide\n");
    output.append("    {\n");
    output.append("    node_t *node = gm_stack_pop(stack);\n");
    output.append(fmt::format("    for (int i = 0; i < {} - 1; ++i) gm_stack_pop(stack);\n", this->n));
    // The top of the stack should now be where the node should go.
    output.append("    node_t **replace_addr = gm_stack_peek(stack, 0);\n");
    output.append("    *replace_addr = node;\n");
    output.append("    }\n");
}

void GInstrBinOp::compile(std::string& output)
{
    output.append(fmt::format("    // GInstrBinOp({})\n", binop_to_name(this->binop)));
    output.append("    {\n");
    output.append("    node_t *n1 = gm_stack_pop(stack);\n");
    output.append("    node_t *n2 = gm_stack_pop(stack);\n");
    switch (this->binop) {
        case GBinop::Add: {
            output.append("    int res = n1->data.number.value + n2->data.number.value;\n");
        } break;
        case GBinop::Sub: {
            output.append("    int res = n1->data.number.value - n2->data.number.value;\n");
        } break;
        case GBinop::Mul: {
            output.append("    int res = n1->data.number.value * n2->data.number.value;\n");
        } break;
        case GBinop::Div: {
            output.append("    int res = n1->data.number.value / n2->data.number.value;\n");
        } break;
    }
    output.append("    node_t *node = gm_alloc_node_number(res);\n");
    output.append("    gm_stack_push(stack, node);\n");
    output.append("    }\n");
}

void GInstrEval::compile(std::string& output)
{
    // BOOKMARK: pg 323
    output.append("    // GInstrEval\n");
    output.append("    {\n");
    output.append("    gm_stack_update_top(stack);\n");
    output.append("    if (stack->top == NULL) RUNTIME_ERROR(\"Stack had a null top during GInstrEval.\");\n");
    output.append("    switch ((*stack->top)->tag) {\n");
    output.append("        case NApply: RUNTIME_ERROR(\"EVAL NOT IMPLEMENTED YET\"); break;\n");
    output.append("        case NFunc: RUNTIME_ERROR(\"EVAL NOT IMPLEMENTED YET\"); break;\n");
    output.append("        default: break; // Do nothing\n");
    output.append("    }\n");
    output.append("    }\n");
    // UNUSED(output);
    // INTERNAL_ERROR("TODO: GInstrEval");
    fmt::println("Warning: EVAL not implemented for Application or Function nodes.");
}

void GInstrAlloc::compile(std::string& output)
{
    output.append("    // GInstrAlloc\n");
    UNUSED(output);
    INTERNAL_ERROR("TODO: GInstrAlloc");
}

void GInstrBegin::compile(std::string& output)
{
    output.append("    // GInstrBegin\n");
    output.append(
        "int main(void) {\n"
        "    stack_t real_stack = {0};\n"
        "    stack_t *stack = &real_stack;\n"
        );
}

void GInstrEnd::compile(std::string& output)
{
    output.append("    // GInstrEnd\n");
    output.append(
        "    gm_stack_free(stack);\n"
        "    return 0;\n"
        "}\n"
        );
}

void GInstrPrint::compile(std::string& output)
{
    output.append("    // GInstrPrint\n");
    output.append("    {\n");
    output.append("    gm_stack_update_top(stack);\n");
    output.append("    if ((*stack->top)->tag == NInteger)");
    output.append("        printf(\"%d\", (*stack->top)->data.number.value);\n");
    output.append("    else RUNTIME_ERROR(\"Printing CONS cells not implemented.\");\n");
    output.append("    }\n");
    // UNUSED(output);
    // INTERNAL_ERROR("TODO: GInstrPrint");
}

void GInstrGlobStart::compile(std::string& output)
{
    output.append("// GInstrGlobStart\n");
    output.append("void gm_global_");
    output.append(this->name);
    output.append("(stack_t *stack) {\n");
    // UNUSED(output);
    // INTERNAL_ERROR("TODO: GInstrGlobStart");
}
