#include "stack.h"
#include <stdio.h>

void stack_init(stack_root_t** stack)
{
    *stack = malloc(sizeof(stack_root_t));
    (*stack)->length = 0;
}


void stack_push(stack_root_t* stack, int value)
{
    if (stack->length > 0) {
        stack->last_node->next_node = malloc(sizeof(stack_node_t));
        stack->last_node->next_node->prev_node = stack->last_node;
        stack->last_node->next_node->next_node = NULL;
        stack->last_node->next_node->value = value;
        stack->last_node = stack->last_node->next_node;
        stack->length++;
    } else {
        stack->first_node = malloc(sizeof(stack_node_t));
        stack->first_node->next_node = NULL;
        stack->first_node->prev_node = NULL;
        stack->first_node->value = value;
        stack->last_node = stack->first_node;
        stack->length++;
    }
}

int stack_pop(stack_root_t* stack)
{
    if (stack->length == 0) return -1;

    int ret_value = stack->last_node->value;
    stack_node_t* tmp = stack->last_node;
    stack->last_node = tmp->prev_node;
    tmp->prev_node = NULL;
    free(tmp);
    stack->length--;
    return ret_value;
}

/*int main(int argc, char const *argv[]) {
    stack_root_t* stack;
    stack_init(&stack);
    stack_push(stack, 1);
    printf("%d\n", stack->last_node->value);
    stack_push(stack, 2);
    printf("%d\n", stack->last_node->value);
    stack_push(stack, 3);
    printf("%d\n", stack->last_node->value);
    printf("%d\n", stack_pop(stack));
    printf("%d\n", stack_pop(stack));
    printf("%d\n", stack_pop(stack));
    // printf("%d\n", *ret);
    free(stack);
    return 0;
}*/
