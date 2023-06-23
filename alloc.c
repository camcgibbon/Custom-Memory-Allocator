/**
 * Malloc
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MIN_ALLOC_SIZE 8

void *calloc(size_t num, size_t size) {
  size_t total_size = num * size;

  void *ptr = malloc(total_size);

  if (ptr)
    memset(ptr, 0, total_size);

  return ptr;
}
typedef struct metadata {
  int size;
  int isUsed;
  struct metadata *next;
  struct metadata *prev; 
  struct metadata *prev_real; 
} metadata_t;

void *startOfHeap = NULL;
metadata_t *endOfHeap = NULL; 
metadata_t *start = NULL; 
metadata_t *end = NULL; 

void *malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    if (startOfHeap == NULL) {
        startOfHeap = sbrk(0);
    }

    metadata_t *current = start;
    metadata_t *prev = NULL;

    while (current != NULL) {
        if (current->size >= size && current->isUsed == 0) {
            break;
        }
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        metadata_t *block = sbrk(sizeof(metadata_t) + size);
        block->size = size;
        block->isUsed = 1;
        block->next = block->prev = NULL;

        if (endOfHeap == NULL) {
            block->prev_real = NULL;
        } else {
            block->prev_real = endOfHeap;
        }

        endOfHeap = block;
        void *ptr = (void *)block + sizeof(metadata_t);
        return ptr;
    }

    splitblock(current, size);
    current->isUsed = 1;
    current->size = size;

    if (current == start) {
        start = current->next;
    } else if (current == end) {
        end = current->prev;
    } else {
        current->prev->next = current->next;
        current->next->prev = current->prev;
    }

    current->prev = current->next = NULL;
    void *ptr = (void *)current + sizeof(metadata_t);
    return ptr;
}

void free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    metadata_t *block = (metadata_t *)((char *)ptr - sizeof(metadata_t));
    block->isUsed = 0;

    if (start == NULL) {
        start = end = block;
        block->prev = block->next = NULL;
    } else {
        metadata_t *prev = end;
        while (prev != NULL && prev->isUsed) {
            prev = prev->prev;
        }
        if (prev == NULL) {
            start = block;
            block->prev = NULL;
        } else {
            block->prev = prev;
            prev->next = block;
        }
        end = block;
        block->next = NULL;
    }

    coalesce(block);
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    metadata_t *current = ptr - sizeof(metadata_t);
    if (current->size >= size) {
        return ptr;
    }

    void *new_ptr = malloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }
    memcpy(new_ptr, ptr, current->size);
    free(ptr);
    return new_ptr;
}

// Helper functions
void splitblock(metadata_t *current, unsigned int desired_size) {
    unsigned const int max = 1;
    unsigned const int can_split = (current->size >= desired_size) && (current->size - desired_size >= max);

    if (can_split) {
        metadata_t *new_block = (metadata_t *)((char *)current + sizeof(metadata_t) + desired_size);
        new_block->isUsed = 0;
        new_block->size = current->size - desired_size - sizeof(metadata_t);
        new_block->prev_real = current;
        new_block->next = current->next;

        if (current->next != NULL) {
            current->next->prev = new_block;
        }

        current->next = new_block;
        new_block->prev = current;

        if (start == NULL) {
            start = current;
        }

        if (current == end) {
            end = new_block;
        }

        current->size = desired_size;

        metadata_t *next_block = (metadata_t *)((char *)new_block + sizeof(metadata_t) + new_block->size);
        if ((void *)next_block < sbrk(0)) {
            next_block->prev_real = new_block;
        }
    }
}


void coalesce(metadata_t *current) {
    metadata_t *ahead = (metadata_t *)((char *)current + current->size + sizeof(metadata_t));
    metadata_t *behind = current->prev_real;
    void *endOfHeap = sbrk(0);
    unsigned int free_ahead = ((void *)ahead < endOfHeap && ahead->isUsed == 0);
    unsigned int free_behind = ((void *)behind >= startOfHeap && behind->isUsed == 0);

    if (free_ahead) {
        current->size += ahead->size + sizeof(metadata_t);

        if (ahead == start) {
            start = ahead->next;
            start->prev = NULL;
        } else {
            ahead->prev->next = ahead->next;
            if (ahead->next != NULL) {
                ahead->next->prev = ahead->prev;
            }
        }

        metadata_t *above_ahead = (metadata_t *)((char *)ahead + ahead->size + sizeof(metadata_t));

        if ((void *)above_ahead < endOfHeap) {
            above_ahead->prev_real = current;
        }

        ahead->size = 0;
        ahead->prev = NULL;
        ahead->prev_real = NULL;
        ahead->next = NULL;
    }

    if (free_behind) {
        behind->size += current->size + sizeof(metadata_t);
        end = current->prev;
        end->next = NULL;

        if ((void *)ahead < endOfHeap) {
            ahead->prev_real = behind;
        }

        current->size = 0;
        current->prev = NULL;
        current->prev_real = NULL;
        current->next = NULL;
    }
}