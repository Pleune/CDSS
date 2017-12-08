#ifndef CDSS_STACK_H
#define CDSS_STACK_H

#include <stdlib.h>

typedef struct stack stack_t;

stack_t *stack_create(size_t object_size, size_t object_count, double resize_factor);
void stack_destroy(stack_t *stack);
void stack_push(stack_t *stack, const void *data);
void *stack_pop(stack_t *stack, void *data);
size_t stack_objects_get_num(stack_t *stack);
void stack_push_mult(stack_t *stack, const void *data, size_t count);
void stack_advance(stack_t *stack, size_t count);
void stack_trim(stack_t *stack);
void stack_resize(stack_t *stack, size_t num_elements);
void stack_ensure_size(stack_t *stack, size_t num_elements);
void *stack_element_ref(stack_t *stack, size_t index);
//delete index by replacing it with the last element
//returns the ref to index, or 0 on error
void *stack_element_replace_from_end(stack_t *stack, size_t index);
void *stack_transform_dataptr(stack_t *stack);

#endif
