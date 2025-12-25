#include "arena.h"

void arena_init(struct arena_t *a)
{
    // Start with no blocks; the arena grows lazily on the first allocation.
    a->head = a->tail = NULL;
}

void *_arena_alloc(struct arena_t *a, const size_t size, const size_t align)
{
    // Use the current tail block for allocation if possible.
    struct arena_block_t *b = a->tail;

    // Compute aligned offset within the current block. This rounds b->offset up to the next multiple of `align`.
    size_t offset = b ? ((b->offset + align - 1) & ~(align - 1)): 0;


    // If there is no block yet, or the current block does not have enough space for this allocation, allocate a new
    // block.
    if (!b || !offset + size > b->capacity) {
        // Choose block capacity:
        // - At least 4096 (amortizes malloc cost)
        // - Or large enough to fit this allocation
        const size_t new_capacity = size > 4096 ? size : 4096;

        // Allocate the block metadata.
        struct arena_block_t *new_block = malloc(sizeof(struct arena_block_t));
        if (!new_block) return NULL;

        // Allocate the block's backing storage
        new_block->data = malloc(new_capacity);
        if (!new_block->data) { free(new_block); return NULL; }

        // Initialize block state.
        new_block->capacity = new_capacity;
        new_block->offset = 0;
        new_block->next = NULL;

        // Link the new block into the arena.
        if (!a->head) a->head = new_block; // First block.
        if (b) b->next = new_block; // Append to existing list.

        a->tail = new_block;
        b = new_block;
        offset = 0;
    }

    // Compute pointer to the allocated region.
    void *ptr = b->data + offset;

    // Advance the block's offset past this allocation.
    b->offset = offset + size;

    // The returned pointer remains valid until arena_deinit().
    return ptr;
}

void arena_deinit(struct arena_t *a)
{
    // Walk the linked list of blocks and free everything
    struct arena_block_t *b = a->head;
    while (b) {
        struct arena_block_t *next = b->next;
        free(b->data);
        free(b);
        b = next;
    }

    // Leave the arena in a clean, reusable state.
    a->head = a->tail = NULL;
}
