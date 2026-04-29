#include "../common.hh"
#include "tree.hh"
#include "types.hh"

/*
  sum (a: int) (b: int) : int

  int -> (int -> int)

  manysum (a: int) (b: int) (c: int) : int

  int -> (int -> (int -> int))
 */

/*
TODO: This function introduced problematic Type allocations
  
Not the best way to do this, maybe reapproach some time?
 */
std::pair<TypePtr, size_t> make_function_type(TreeParamsNode *node, TypePtr ret_type)
{
    std::vector<TypePtr> types;
    for (auto& child : node->children) {
        TreeIdentNode *param = static_cast<TreeIdentNode *>(child.get());
        assert(param->attr_type.has_value());
        types.push_back(*param->attr_type);
    }

    // Avoid the unnecessary copy for an alias
    TypePtr& fn_type = ret_type;

    size_t arity = types.size();
    while (!types.empty()) {
        TypePtr arg = types.back();
        fn_type = std::make_shared<Type>(TypeFunction(arg, fn_type));
        types.pop_back();
    }
    return std::make_pair(fn_type, arity);
}
