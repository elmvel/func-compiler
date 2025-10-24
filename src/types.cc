#include "common.hh"
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
std::pair<Type *, size_t> make_function_type(TreeParamsNode *node, Type *ret_type)
{
    std::vector<Type *> types;
    for (auto& child : node->children) {
        TreeIdentNode *param = static_cast<TreeIdentNode *>(child.get());
        assert(param->attr_type.has_value());
        types.push_back(*param->attr_type);
    }

    Type *fn_type = ret_type;
    size_t arity = types.size();
    while (!types.empty()) {
        Type *arg = types.back();
        fn_type = new Type(TypeFunction(arg, fn_type));
        types.pop_back();
    }
    return std::make_pair(fn_type, arity);
}
