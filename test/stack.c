#include "cdss/cdss_all.h"

int
test_stack_basic(void)
{
    stack_t *stack = stack_create(sizeof(int), 2, 2.0);
    if(stack_objects_get_num(stack) != 0)
        return 1;

    int i;
    int j;

    for(i=0; i<10000; i++)
        stack_push(stack, &i);

    if(stack_objects_get_num(stack) != 10000)
        return 1;

    for(i=0; i<10000; i++)
    {
        stack_pop(stack, &j);
        if(j != 9999-i)
            return 1;
    }

    stack_destroy(stack);

    return 0;
}

int
test_stack_advanced(void)
{
    stack_t *stack = stack_create(sizeof(int), 100, 1.5);

    int i;

    stack_ensure_size(stack, 10000);

    for(i=0; i<10000; i++)
        *(int *)(stack_element_ref(stack, i)) = i;

    stack_advance(stack, 10000);

    for(i=0; i<10000; i++)
    {
        int j;
        stack_pop(stack, &j);
        if(j != 9999-(int)i)
            return 1;
    }

    int *data = stack_transform_dataptr(stack);
    stack = stack_create(sizeof(int), 100, 2.0);
    stack_trim(stack);

    stack_push_mult(stack, data, 10000);

    for(i=0; i<10000; i++)
    {
        int j;
        stack_pop(stack, &j);
        if(j != 9999-(int)i)
            return 1;
    }

    for(i=0; i<10000; i++)
        stack_push(stack, &i);

    for(i=0; i<9999; i++)
    {
        stack_element_replace_from_end(stack, 0);
        if(*(int *)stack_element_ref(stack, 0) != 9999 - i)
            return 1;
    }

    if(stack_objects_get_num(stack) != 1)
        return 1;

    if(*(int *)stack_element_ref(stack, 0) != 1)
        return 1;

    for(i=0; i<9999; i++)
        stack_push(stack, &i);

    stack_resize(stack, 100);

    if(stack_objects_get_num(stack) != 100)
        return 1;

    free(data);
    stack_destroy(stack);

    return 0;
}

int
main()
{
    return test_stack_basic() || test_stack_advanced();
}
