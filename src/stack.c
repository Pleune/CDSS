#include "cdss/stack.h"
#include "cdss/minmax.h"

#include <string.h>
#include <stdio.h>

struct stack {
	unsigned char *data;
	unsigned char *top; //non inclusive
	unsigned char *end; //non inclusive

	size_t object_size;
	size_t size;

	double resize_factor;
};

inline static int
resize(struct stack *stack, size_t size)
{
	size_t offset = stack->top - stack->data;

	stack->size = size;
	unsigned char *new_data = realloc(stack->data, size);
	if(!new_data)
		return 1;

    stack->data = new_data;
	stack->top = offset + stack->data;
	stack->end = stack->data + size;

    if(stack->top > stack->end)
        stack->top = stack->end;

    return 0;
}

struct stack *
stack_create(size_t object_size, size_t object_count, double resize_factor)
{
	struct stack *stack = malloc(sizeof(struct stack));
    if(!stack) return 0;

	stack->object_size = object_size;
	stack->size = object_count * object_size;

	stack->data = malloc(stack->size);
	if(!stack->data)
    {
        free(stack);
        return 0;
    }

	stack->top = stack->data;
	stack->end = stack->data + stack->size;

	stack->resize_factor = resize_factor;

	return stack;
}

void
stack_destroy(struct stack *stack)
{
	free(stack->data);
	free(stack);
}

void
stack_resize(struct stack *stack, size_t num_elements)
{
	resize(stack, num_elements * stack->object_size);
}

void
stack_ensure_size(stack_t *stack, size_t num_elements)
{
	size_t size_after = stack->size;

	while(size_after < num_elements * stack->object_size)
		size_after *= stack->resize_factor;

	if(size_after > stack->size)
		resize(stack, size_after);
}

size_t
stack_objects_get_num(struct stack *stack)
{
	return (stack->top - stack->data) / stack->object_size;
}

void
stack_trim(struct stack *stack)
{
	size_t size = MAX((size_t)(stack->top - stack->data), stack->object_size);
	resize(stack, size);
}

void
stack_push(struct stack *stack, const void *data)
{
	stack_ensure_size(stack, stack->top - stack->data + stack->object_size);
	memcpy(stack->top, data, stack->object_size);
	stack->top += stack->object_size;
}

void *
stack_pop(struct stack *stack, void *data)
{
	if(stack->top == stack->data)
		return 0;

	stack->top -= stack->object_size;
	memcpy(data, stack->top, stack->object_size);

	return data;
}

void *
stack_element_ref(stack_t* stack, size_t index)
{
	if(index >= stack->size / stack->object_size)
		return 0;

	void *ref = stack->data + stack->object_size * index;
	return ref;
}

void *
stack_element_replace_from_end(stack_t* stack, size_t index)
{
	void *ref;
	if((ref = stack_element_ref(stack, index)) == 0)
		return 0;

	stack_pop(stack, ref);
	return ref;
}

void
stack_push_mult(struct stack* stack, const void* data, size_t count)
{
	stack_ensure_size(stack, (stack->top - stack->data) + stack->object_size*count);
	memcpy(stack->top, data, stack->object_size * count);
	stack->top += stack->object_size * count;
}

void
stack_advance(stack_t *stack, size_t count)
{
	stack_ensure_size(stack, (stack->top - stack->data) + stack->object_size*count);
	stack->top += stack->object_size * count;
}

void *
stack_transform_dataptr(stack_t *stack)
{
	void *ret = stack->data;
	free(stack);
	return ret;
}
