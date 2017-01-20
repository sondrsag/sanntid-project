#include "stack.h"
#include <stdio.h>

void stackInit(stack_root_t** stack)
{
    *stack = malloc(sizeof(stack_root_t));
    (*stack)->length = 0;
}


void stackPush(stack_root_t* stack, int value)
{
    if (stack->length > 0) {
        stack_node_t* node = stack->first_node;

        while (node->next_node != NULL) {
            node = node->next_node;
        }

        node->next_node = malloc(sizeof(stack_node_t));
        node = node->next_node;
        node->value = value;
        node->next_node = NULL;
        stack->length++;
    } else {
        stack->first_node = malloc(sizeof(stack_node_t*));
        stack->first_node->value = value;
        stack->first_node->next_node = NULL;
        stack->length++;
    }
}

int stackPop(stack_root_t* stack)
{
    if (stack->length == 0) { return -1; }

    if (stack->length == 1) {
        int ans = stack->first_node->value;
        free(stack->first_node);
        stack->first_node = NULL;
        stack->length--;
        return ans;
    }

    stack_node_t* node = stack->first_node;

    while (node->next_node->next_node != NULL) {
        node = node->next_node;
    }

    int ans = node->next_node->value;
    free(node->next_node);
    node->next_node = NULL;
    stack->length--;
    return ans;
}

bool stackIsEmpty(stack_root_t* stack)
{
    return stack->length == 0;
}

/*int main(int argc, char const *argv[]) {
    stack_root_t* stack;
    stackInit(&stack);
    stackPush(stack, 1);
    stackPush(stack, 2);
    stackPush(stack, 3);
    printf("%d\n", stackPop(stack));
    printf("%d\n", stackPop(stack));
    printf("%d\n", stackPop(stack));
    printf("%d\n", stackPop(stack));
    stackPush(stack, 1);
    stackPush(stack, 2);
    stackPush(stack, 3);
    stackPush(stack, 4);
    stackPush(stack, 5);
    printf("%d\n", stackPop(stack));
    printf("%d\n", stackPop(stack));
    printf("%d\n", stackPop(stack));
    printf("%d\n", stackPop(stack));
    printf("%d\n", stackPop(stack));
    printf("%d\n", stackPop(stack));
    free(stack);
    return 0;
}*/
