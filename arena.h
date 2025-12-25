#ifndef ARENA_H
#define ARENA_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * @file arena.h
 * @brief Simple arena (region-based) memory allocator.
 * @details This allocator provides fast, linear allocations from one or more internally managed memory blocks.
 * Individual allocations cannot be freed; all memory is released at once via arena_deinit().\n
 * \n
 * Basic usage:
 * @code{c}
 * #include <stdio.h>
 * #include <string.h>
 *
 * #include "arena.h"
 *
 * typedef struct node {
 *     int value;
 *     struct node *next;
 * } node_t;
 *
 * typedef struct {
 *     char *name;
 *     int *scores;
 *     size_t score_count;
 * } user_t;
 *
 * int main(void) {
 *     struct arena_t arena;
 *     arena_init(&arena);
 *
 *     // Allocate a linked list.
 *     node_t *head = arena_alloc(&arena, node_t);
 *     head->value = 1;
 *
 *     head->next = arena_alloc(&arena, node_t);
 *     head->next->value = 2;
 *
 *     head->next->next = NULL;
 *
 *     // Allocate a struct with dynamic members.
 *     user_t *user = arena_alloc(&arena, user_t);
 *     user->name = arena_alloc_array(&arena, char, 4); // Includes '\0'.
 *     memcpy(user->name, "Bob", 4);
 *
 *     user->score_count = 3;
 *     user->scores = arena_alloc_array(&arena, int, user->score_count);
 *     user->scores[0] = 90;
 *     user->scores[1] = 85;
 *     user->scores[2] = 92;
 *
 *     // Use the data.
 *     printf("User: %s\n", user->name);
 *     for (node_t *n = head; n; n = n->next) printf("Node value: %d\n", n->value);
 *
 *     // One call frees everything!
 *     arena_deinit(&arena);
 *     return 0;
 * }
 * @endcode
 * \n
 * While it's totally possible to safely use _arena_alloc(), it's recommended to use the arena_alloc() and
 * arena_alloc_array() macros which automatically compute the correct size, alignment, and cast from the type you name.
 * This is much safer, more convenient, and less error-prone; eliminating the most common sources of arena-allocation
 * bugs.
**/

/**
 * @def arena_alloc
 * @brief Allocate a single object of the given type from the arena.
 *
 * @param a    Pointer to an initialized arena.
 * @param type Type to allocate.
 *
 * @return Pointer to the allocated object
 *
 * @note The returned memory is aligned according to the alignment requirements of @p type.
**/
#define arena_alloc(a, type) ((type*)_arena_alloc(a, sizeof(type), _Alignof(type)))

/**
 * @def arena_alloc_array
 * @brief Allocate an array of objects of the given type from the arena.
 *
 * @param a     Pointer to an initialized arena.
 * @param type  Element type.
 * @param count Number of elements to allocate.
 *
 * @return Pointer to the first element of the allocated array.
 *
 * @note The returned memory is aligned according to the alignment requirements of @p type.
**/
#define arena_alloc_array(a, type, count) ((type*)_arena_alloc(a, sizeof(type)*(count), _Alignof(type)))

/**
 * @struct arena_block_t
 * @brief Internal memory block used by the arena allocator.
 * @details Each block owns a contiguous chunk of memory from which allocations are linearly served. Blocks are linked
 * together when the arena grows.
 *
 * @note This structure is considered an implementation detail.
**/
struct arena_block_t {
    uint8_t *data;              /**< Pointer to the raw memory buffer */
    size_t capacity;            /**< Total size of the buffer in bytes */
    size_t offset;              /**< Current allocation offset */
    struct arena_block_t *next; /**< Next block in the chain */
};

/**
 * @struct arena_t
 * @brief Arena allocator state.
 * @details An arena manages one or more memory blocks and serves aligned, linear allocations. All allocations remain
 * valid until the arena is deinitialized.
**/
struct arena_t {
    struct arena_block_t *head; /**< First block in the arena */
    struct arena_block_t *tail; /**< Current block used for allocation */
};

/**
 * @brief Initialize an arena.
 * @details The arena must be initialized before calling _arena_alloc(). After initialization, the arena owns no memory
 * until the first allocation request.
 *
 * @param a A pointer to the arena to initialize.
**/
void arena_init(struct arena_t *a);

/**
 * @brief Allocate memory from the arena.
 *
 * @param a     Pointer to an initialized arena.
 * @param size  Number of bytes to allocate.
 * @param align Required alignment in bytes (must be a power of two)
 *
 * @return Pointer to the beginning of the allocated memory, or @c NULL if allocation fails.
 *
 * @note The returned memory remains valid until arena_deinit() is called. Individual allocations cannot be freed.
**/
void *_arena_alloc(struct arena_t *a, size_t size, size_t align);

/**
 * @brief Deinitialize an arena and free all associated memory.
 *
 * @param a Pointer to the arena to deinitialize.
 *
 * @note This releases all memory blocks owned by the arena. All pointers previously returned by _arena_alloc() become
 * invalid.
 */
void arena_deinit(struct arena_t *a);

#endif /* ARENA_H */
