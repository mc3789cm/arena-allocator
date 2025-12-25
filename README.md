# The Arena Allocator
A small, simple, fast, and dependency-free *arena memory allocator* for C.

This allocation concept is for workloads with a clear allocation lifetime (e.g. parsing, request handling, AST
construction). The arena provides fast, linear allocations from one or more internally managed memory blocks where many
small allocations can be freed all at once; individual allocations cannot be freed.

A [Doxygen](https://www.doxygen.nl/) style API reference is include in [`arena.h`](./arena.h).

## Features
- Extremely fast allocation
- Correct alignment for all C types
- Grows automatically using linked blocks
- No per-allocation `free()`
- Zero external dependencies

## When to Use an Arena
Arena allocation is ideal when:
- You perform many small allocations
- All allocations share the same lifetime
- You want deterministic, O(1) cleanup

Common use cases:

- JSON/protocol parsing
- Compilers and interpreters
- Game engines
- Request/response lifetimes
- Temporary object graphs

## When not to Use an Arena
Arena allocation is not ideal when:
- If objects *must* be freed individually
- If memory pressure matters during runtime
- You need precise memory ownership
- You need reallocation semantics
- Multithreaded shared allocation (unless you use a thread-local arena)

**Rule of Thumb**: Use an arena when you know *when everything can die*, and avoid it when you don't.

## Basic Usage
```c
#include <stdio.h>
#include <string.h>

#include "arena.h

typedef struct node {
    int value;
    struct node *next;
} node_t;

typedef struct {
    char *name;
    int *scores;
    size_t score_count;
} user_t;

int main(void) {
    struct arena_t arena;
    arena_init(&arena);

    // Allocate a linked list.
    node_t *head = arena_alloc(&arena, node_t);
    head->value = 1;

    head->next = arena_alloc(&arena, node_t);
    head->next->value = 2;

    head->next->next = NULL;

    // Allocate a struct with dynamic members.
    user_t *user = arena_alloc(&arena, user_t);
    user->name = arena_alloc_array(&arena, char, 4); // Includes '\0'.
    memcpy(user->name, "Bob", 4);

    user->score_count = 3;
    user->scores = arena_alloc_array(&arena, int, user->score_count);
    user->scores[0] = 90;
    user->scores[1] = 85;
    user->scores[2] = 92;

    // Use the data.
    printf("User: %s\n", user->name);
    for (node_t *n = head; n; n = n->next) printf("Node value: %d\n", n->value);

    // One call frees everything!
    arena_deinit(&arena);
    return 0;
}
```

## License
Unlicensed license. See [`LICENSE`](./LICENSE).
