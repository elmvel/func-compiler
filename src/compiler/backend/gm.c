#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*
  Here rests all of the runtime code for the functional language.
*/

#define RUNTIME_ERROR(...)                     \
    do {                                        \
        fprintf(stderr, "runtime error: ");     \
        fprintf(stderr, __VA_ARGS__);           \
        fprintf(stderr, "\n");                  \
        exit(1);                                \
    } while (0)

/*
G-Machine Specs:
S - The stack
G - The graph
C - The G-code sequence remaining to be run
D - The dump
  * A stack of pairs (S, C) where s=stack,c=code seq
G-Machine = tuple <S, G, C, D>
  
G-Code Instructions:
*/

typedef enum {
    NInteger,
    NCons,
    NApply,
    NFunc, // supercombinator or built-in
    NHole, // node to be filled in later
    NIndirection, // Pointer to another node
} node_k;

typedef struct node_t node_t;

typedef struct {
    int value;
} node_int_t;

typedef struct {
    node_t *fun;
    node_t *arg;
} node_apply_t;

typedef struct {
    const char *name;
} node_func_t;

typedef struct {
    node_t *ind_node;
} node_ind_t;

typedef struct {
    node_k tag;
    union {
        node_int_t number;
        node_apply_t app;
        node_func_t func;
        node_ind_t ind;
    } data;
} node_t;

node_t *gm_alloc_node()
{
    node_t *node = malloc(sizeof(node_t));
    memset(node, 0, sizeof(node_t));
    return node;
}

node_t *gm_alloc_node_number(int value)
{
    node_t *node = gm_alloc_node();
    node->tag = NInteger;
    node->data.number.value = value;
    return node;
}

node_t *gm_alloc_node_app(node_t *fun, node_t *app)
{
    node_t *node = gm_alloc_node();
    node->tag = NApply;
    node->data.app.fun = fun;
    node->data.app.arg = arg;
    return node;
}

node_t *gm_alloc_node_func(const char *name)
{
    node_t *node = gm_alloc_node();
    node->tag = NFunc;
    node->data.func.name = name;
    return node;
}

node_t *gm_alloc_node_ind(node_t *n)
{
    node_t *node = gm_alloc_node();
    node->tag = NIndirection;
    node->data.ind.ind_node = n;
    return node;
}

typedef struct {
    node_t **items;
    size_t count;
    size_t capacity;
    node_t **top;
} stack_t;

#define GM_STACK_CAP 128

void gm_stack_free(stack_t *stack)
{
    free(stack->items);
    memset(stack, 0, sizeof(stack_t));
}

void gm_stack_update_top(stack_t *stack)
{
    if (stack->count > 0) {
        stack->top = &stack->items[stack->count - 1];
    } else {
        stack->top = NULL;
    }
}

void gm_stack_push(stack_t *stack, node_t *node)
{
    if (stack->count >= stack->capacity) {
        stack->capacity = stack->capacity == 0 ? GM_STACK_CAP : stack->capacity * 2;
        stack->items = realloc(stack->items, stack->capacity);
        assert(stack->items != NULL && "Out of memory");
    }
    stack->items[stack->count++] = node;
    gm_stack_update_top(stack);
}

node_t *gm_stack_pop(stack_t *stack)
{
    if (stack->count > 0) {
        node_t *popped = *(stack->top);
        stack->count -= 1;
        gm_stack_update_top(stack);
        return popped;
    } else {
        RUNTIME_ERROR("Tried to pop off of an empty stack.");
    }
}

node_t **gm_stack_peek(stack_t *stack, size_t offset)
{
    if (offset < stack->count) {
        return &stack->items[stack->count - 1 - offset];
    } else {
        RUNTIME_ERROR("Peeked below the size of the stack.");
    }
}

node_t *gm_stack_slide(stack_t *stack, size_t n)
{
    assert(0 && "TODO: gm_stack_slide");
}

// TODO: represent the dump and the code instruction sequence in some way?
typedef struct dump_t dump_t;
typedef struct code_seq_t code_seq_t;
