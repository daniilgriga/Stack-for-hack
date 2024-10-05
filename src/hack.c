#define CANARY_PROTECTION
#define DEBUG
#define HASH

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "color_print.h"

#ifdef CANARY_PROTECTION
    #define ON_CNR_PRTCT(...) __VA_ARGS__
#else
    #define ON_CNR_PRTCT(...)
#endif

#ifdef DEBUG
    #define ON_DBG(...) __VA_ARGS__
#else
    #define ON_DBG(...)
#endif

#ifdef HASH
    #define ON_HASH(...) __VA_ARGS__
#else
    #define ON_HASH(...)
#endif

#ifdef DEBUG
    #define STACK_ASSERT(...) stack_assert(__VA_ARGS__)
#else
    #define STACK_ASSERT(...)
#endif

typedef unsigned long hash_t;
typedef double stack_elem_t;

#pragma GCC diagnostic ignored "-Wlarger-than=8192"
#pragma GCC diagnostic ignored "-Wstack-protector"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough="

struct stack_str
{
    ON_CNR_PRTCT (double canary_stack_1;)

    stack_elem_t* data;
    int size;
    int capacity;

    ON_HASH ( hash_t hash_st;
              hash_t hash_b;  )

    ON_CNR_PRTCT (double canary_stack_2;)
};

ON_DBG (struct test_str
{
    char executioner_1[1];
    struct stack_str stack;
    char executioner_2[1];
};  )

enum StackCondition
{
    STACK_IS_OK,
    STACK_IS_BAD
};

enum errors
{
    STACK_OK                    = 0,
    STACK_IS_NULL               = 1,
    STACK_DATA_IS_NULL          = 2,
    STACK_BAD_SIZE              = 4,
    STACK_SIZE_IS_LESS_CAPACITY = 8,
    STACK_CANARY_PROBLEM        = 16,
    STACK_DATA_CANARY_PROBLEM   = 32,
    STACK_HASH_PROBLEM          = 64,
    STACK_DATA_HASH_PROBLEM     = 128
};

const stack_elem_t STACK_POISON_ELEM = -666.13;

const double CANARY_VALUE        = (0xBAD);
const double CANARY_BUFFER_VALUE = (0xEDA);

int stack_error (struct stack_str* stack);

int error_code_output (int err);

int convert_to_binary (int n);

enum StackCondition stack_ctor (struct stack_str* stack, int capacity, const char* file, int line, const char* func);

int stack_assert (struct stack_str* stack, const char* file, int line, const char* func);

int stack_dtor (struct stack_str* stack);

int stack_push (struct stack_str* stack, stack_elem_t elem ON_DBG(, const char* file, int line, const char* func));

int stack_pop (struct stack_str* stack, stack_elem_t* x ON_DBG(, const char* file, int line, const char* func));

int stack_dump (struct stack_str* stack, const char* file, int line, const char* func);

bool stack_ok (const struct stack_str* stack);

int increasing_capacity (struct stack_str* stack ON_DBG(, const char* file, int line, const char* func));

int torture_stack (struct stack_str* stack, int number_canary_check ON_DBG(, const char* file, int line, const char* func));

void printf_place_info (ON_DBG(const char* file, int line, const char* func, const char* string));

hash_t hash_djb2(const char* str, size_t size_of_stack);

int convert_to_binary_upd (unsigned n);

int calculate_hash (struct stack_str* stack);

int main (void)
{
    struct stack_str stack = {};

    if (stack_ctor(&stack, 2,  __FILE__, __LINE__, __FUNCTION__) == STACK_IS_BAD)
    {
        printf("error in stack_ctor, check the data of stack, bro.\n\n");
        return 0;
    }

    stack_dump(&stack, __FILE__, __LINE__, __FUNCTION__);

    stack_push (&stack, 10 ON_DBG(,  __FILE__, __LINE__, __FUNCTION__));
    stack_push (&stack, 20 ON_DBG(,  __FILE__, __LINE__, __FUNCTION__));
    stack_push (&stack, 30 ON_DBG(,  __FILE__, __LINE__, __FUNCTION__));
    stack_push (&stack, 40 ON_DBG(,  __FILE__, __LINE__, __FUNCTION__));
    stack_push (&stack, 50 ON_DBG(,  __FILE__, __LINE__, __FUNCTION__));
    stack_push (&stack, 60 ON_DBG(,  __FILE__, __LINE__, __FUNCTION__));

    stack_elem_t x = 0;

    stack_pop(&stack, &x ON_DBG(,  __FILE__, __LINE__, __FUNCTION__));

    stack_dump(&stack, __FILE__, __LINE__, __FUNCTION__);

    ON_CNR_PRTCT ( torture_stack(&stack, 0 ON_DBG(,  __FILE__, __LINE__, __FUNCTION__)); )

    stack_dtor(&stack);

    return 0;
}

enum StackCondition stack_ctor (struct stack_str* stack, int capacity, const char* file, int line, const char* func)
{
    assert(stack);

    ON_CNR_PRTCT ( stack->canary_stack_1 = CANARY_VALUE;
                   stack->canary_stack_2 = CANARY_VALUE; )

    stack->size = 0;

    stack->data = ON_CNR_PRTCT(1 +) (stack_elem_t*) calloc((size_t) capacity ON_CNR_PRTCT(+ 2), sizeof(stack->data[0]));

    stack->capacity = capacity;

    ON_CNR_PRTCT ( memcpy(&stack->data[-1],              &CANARY_BUFFER_VALUE, sizeof(CANARY_BUFFER_VALUE));
                   memcpy(&stack->data[stack->capacity], &CANARY_BUFFER_VALUE, sizeof(CANARY_BUFFER_VALUE)); )

    for (int i = 0; i < stack->capacity; i++)
            stack->data[stack->size + i] = STACK_POISON_ELEM;

    ON_HASH ( calculate_hash(stack); )

    STACK_ASSERT(stack, file, line, func);

    return STACK_IS_OK;
}

int stack_dtor (struct stack_str* stack)
{
    for (int i = 0; i < stack->capacity; i++)
        stack->data[i] = STACK_POISON_ELEM;

    free(stack->data ON_CNR_PRTCT (- 1)); stack->data = NULL;

    stack->size = 0;

    stack->capacity = 0;

    return 0;
}

int increasing_capacity (struct stack_str* stack ON_DBG(, const char* file, int line, const char* func))
{
    if (stack->size >= stack->capacity)
    {
        stack->capacity *= 2;

        stack->data = ON_CNR_PRTCT(1 +) (stack_elem_t*) realloc (stack->data ON_CNR_PRTCT(- 1), (size_t) (stack->capacity ON_CNR_PRTCT(+ 2)) * sizeof(stack->data[0]));

        for (int i = 0; i < stack->size; i++)
            stack->data[stack->size + i] = STACK_POISON_ELEM;

        ON_CNR_PRTCT ( memcpy(&stack->data[stack->capacity], &CANARY_BUFFER_VALUE, sizeof(CANARY_BUFFER_VALUE)); )

        ON_HASH ( calculate_hash(stack); )

        STACK_ASSERT(stack, file, line, func);
    }

    return 0;
}

int stack_push (struct stack_str* stack, stack_elem_t elem ON_DBG(, const char* file, int line, const char* func))
{

    increasing_capacity (stack ON_DBG(, file, line, func));

    stack->data[stack->size++] = elem;

    ON_HASH ( calculate_hash(stack); )

    STACK_ASSERT(stack, file, line, func);

    return 0;
}

int calculate_hash (struct stack_str* stack)
{
    stack->hash_b  = hash_djb2((const char*)(stack->data ON_CNR_PRTCT(- 1)), (size_t)stack->capacity);
    stack->hash_st = 0;
    stack->hash_st = hash_djb2((const char*)stack, sizeof(*stack));

    return 0;
}

int stack_pop (struct stack_str* stack, stack_elem_t* x ON_DBG(, const char* file, int line, const char* func))
{
    STACK_ASSERT(stack, file, line, func);

    *x = stack->data[stack->size - 1];
    stack->data[--stack->size] = STACK_POISON_ELEM;

    ON_HASH ( calculate_hash(stack); )

    return 0;
}

int stack_dump (struct stack_str* stack, const char* file, int line, const char* func)
{
    printf(PURPLE_TEXT("\nstack_str [%p] called from %s, name \"%s\" born at %s:%d:\n"), stack, file, "stack", func, line);

    if (stack == NULL)
    {
        printf("stack = "BLUE_TEXT("[%p]")", this is all the information I can give\n\n", stack);
        return 0;
    }
    ON_HASH      ( printf(  "hash_st = "BLUE_TEXT("%x")"\n", (unsigned int)stack->hash_st);
                   printf(  "hash_b  = "BLUE_TEXT("%x")"\n", (unsigned int)stack->hash_b ); )
    ON_CNR_PRTCT ( printf("\ncanary_1 in stack   = "BLUE_TEXT("0x%x")"\n\n" , (unsigned int)stack->canary_stack_1); )
                   printf(  "stack->capacity     = "BLUE_TEXT("%d")"\n",      stack->capacity);
                   printf(  "stack->size         = "BLUE_TEXT("%d")"\n",      stack->size);
                   printf(  "stack->data address = "BLUE_TEXT("[%p]")"\n\n",  stack->data);
    ON_CNR_PRTCT ( printf(  "canary_2 in stack   = "BLUE_TEXT("0x%x")"\n\n" , (unsigned int)stack->canary_stack_2); )

    if (stack->data == NULL)
    {
        printf("stack->data = "BLUE_TEXT("[%p]")", this is all the information I can give", stack->data);
        return 0;
    }

    for (int i = 0; i < stack->capacity; i++)
    {
        if (memcmp(&stack->data[i], &STACK_POISON_ELEM, sizeof(STACK_POISON_ELEM)) == 0)
            printf("stack->data[%d] = "BLUE_TEXT("%g (POISON)")"\n", i, stack->data[i]);
        else
            printf("stack->data[%d] = "BLUE_TEXT("%g")"\n", i, stack->data[i]);
    }

    ON_CNR_PRTCT ( printf("\ncanary_3 in buffer = "BLUE_TEXT("0x%x")"\n\n", (unsigned int)stack->data[-1]);
                   printf("canary_4 in buffer = "BLUE_TEXT("0x%x")"\n\n", (unsigned int)stack->data[stack->capacity]); )

    return 0;
}

int stack_error (struct stack_str* stack)
{
    int err = 0;

    if (stack == NULL)
        err |= STACK_IS_NULL;

    if (stack->data == NULL)
        err |= STACK_DATA_IS_NULL;

    if (stack->size < 0)
        err |= STACK_BAD_SIZE;

    if ((stack->size) > (stack->capacity))
        err |= STACK_SIZE_IS_LESS_CAPACITY;

    ON_CNR_PRTCT ( if (memcmp(&stack->canary_stack_1, &CANARY_VALUE, sizeof(CANARY_VALUE)) ||
                       memcmp(&stack->canary_stack_2, &CANARY_VALUE, sizeof(CANARY_VALUE)))
                       err |= STACK_CANARY_PROBLEM;

                   if (memcmp(&stack->data[-1],              &CANARY_BUFFER_VALUE, sizeof(CANARY_BUFFER_VALUE)) &&
                       memcmp(&stack->data[stack->capacity], &CANARY_BUFFER_VALUE, sizeof(CANARY_BUFFER_VALUE)))
                       err |= STACK_DATA_CANARY_PROBLEM; )

    ON_HASH    ( hash_t old_hash_st = stack->hash_st;
                 stack->hash_st = 0;
                 stack->hash_st = hash_djb2((const char*)stack, sizeof(*stack));

                 if (old_hash_st != stack->hash_st)
                     err |= STACK_HASH_PROBLEM;

                 stack->hash_st = old_hash_st;

                 hash_t old_hash_b = stack->hash_b;
                 stack->hash_b = 0;
                 stack->hash_b  = hash_djb2((const char*)(stack->data ON_CNR_PRTCT(- 1)), (size_t)stack->capacity * sizeof (stack->data[0]));

                 if (old_hash_b == stack->hash_b)
                     err |= STACK_DATA_HASH_PROBLEM;

                 stack->hash_b = old_hash_b; )

    return err;
}

int error_code_output (int err)
{
    printf(RED_TEXT("ERROR: "));

    convert_to_binary(err);

    switch (err)
    {
        case STACK_IS_NULL:
            printf(RED_TEXT(" - STACK_IS_NULL"));
            break;
        case STACK_DATA_IS_NULL:
            printf(RED_TEXT(" - STACK_DATA_IS_NULL"));
            break;
        case STACK_BAD_SIZE:
            printf(RED_TEXT(" - STACK_BAD_SIZE"));
            break;
        case STACK_SIZE_IS_LESS_CAPACITY:
            printf(RED_TEXT(" - STACK_SIZE_IS_LESS_CAPACITY"));
            break;
        case STACK_CANARY_PROBLEM:
            printf(RED_TEXT(" - STACK_CANARY_PROBLEM"));
            break;
        case STACK_DATA_CANARY_PROBLEM:
            break;
            printf(RED_TEXT(" - STACK_DATA_CANARY_PROBLEM"));
        case STACK_HASH_PROBLEM:
            printf(RED_TEXT(" - STACK_HASH_PROBLEM"));
            break;
        case STACK_DATA_HASH_PROBLEM:
            printf(RED_TEXT(" - STACK_DATA_HASH_PROBLEM"));
            break;
        default:
            printf(RED_TEXT(" - UNKNOWN ERROR"));
    }

    printf("\n");

    return 0;
}

int convert_to_binary (int n)
{
    if (n > 1) convert_to_binary(n / 2);

    printf(RED_TEXT("%d"), (int) n % 2);

    return 0;
}

int convert_to_binary_upd (unsigned n)
{
    for (int i = 0; i < 32; i++)
        {
        printf ("%c", (int)((n & 1) + '0'));
        n >>= 1;
        }

    return 0;
}


int stack_assert (struct stack_str* stack, const char* file, int line, const char* func)
{
    if (!stack_error(stack))
        return 0;

    printf_place_info(file, line, func, "STACK_ASSERT");

    fprintf(stderr, RED_TEXT("STACK IS NOT OKAY:\n"));

    error_code_output(stack_error(stack));

    if (stack == NULL)
        fprintf(stderr, YELLOW_TEXT("stack = NULL\n"));

    if (stack->data == NULL)
        fprintf(stderr, YELLOW_TEXT("stack->data = NULL\n"));

    if ((stack->size) > (stack->capacity))
        fprintf(stderr, YELLOW_TEXT("stack->size = %d | stack->capacity = %d\n"), stack->size, stack->capacity);

    ON_CNR_PRTCT ( if (memcmp(&stack->canary_stack_1, &CANARY_VALUE, sizeof(CANARY_VALUE)) ||
                       memcmp(&stack->canary_stack_2, &CANARY_VALUE, sizeof(CANARY_VALUE)))
                       fprintf(stderr, YELLOW_TEXT("stack->canary_stack_1 = %x | stack->canary_stack_2 = %x\n"),
                              (unsigned int)stack->canary_stack_1, (unsigned int)stack->canary_stack_2);

                   if (memcmp(&stack->data[-1], &CANARY_BUFFER_VALUE, sizeof(CANARY_BUFFER_VALUE)) &&
                       memcmp(&stack->data[stack->capacity], &CANARY_BUFFER_VALUE, sizeof(CANARY_BUFFER_VALUE)))
                       fprintf(stderr, YELLOW_TEXT("canary_buffer_3 = %x | canary_buffer_4 = %x\n"),
                              (unsigned int)stack->data[-1], (unsigned int)stack->data[stack->capacity]); )


    ON_HASH    ( hash_t old_hash_st = stack->hash_st;
                 stack->hash_st = 0;
                 stack->hash_st = hash_djb2((char*)stack, sizeof(*stack));

                 if (old_hash_st != stack->hash_st)
                     fprintf(stderr, YELLOW_TEXT("old_hash_st = %x | stack->hash_st = %x\n"),
                             (unsigned int)old_hash_st, (unsigned int)stack->hash_st);

                 stack->hash_st = old_hash_st;

                 hash_t old_hash_b = stack->hash_b;
                 stack->hash_b = 0;
                 stack->hash_b  = hash_djb2((char*)(stack->data ON_CNR_PRTCT(- 1)), (size_t)stack->capacity * sizeof (stack->data[0]));

                 if (old_hash_b == stack->hash_b)
                     fprintf(stderr, YELLOW_TEXT("old_hash_b = %x | stack->hash_b = %x\n"),
                             (unsigned int)old_hash_b, (unsigned int)stack->hash_b); )

                 stack->hash_b = old_hash_b;

    assert(0);

    return 0;
}

void printf_place_info (const char* file, int line, const char* func, const char* string)
{
    printf(PURPLE_TEXT("called from %s, name \"%s\" born at %s:%d:\n"),  file, string, func, line);
}

int torture_stack (struct stack_str* stack, int number_canary_check ON_DBG(, const char* file, int line, const char* func))
{
    if (number_canary_check == 0)
        return 0;

    if (number_canary_check == 1)
        memset(stack, 0, sizeof(*stack));

    if (number_canary_check == 3)
        stack->data[-1] = 111111;

    if (number_canary_check == 4)
        stack->data[stack->capacity] = 111111;

    printf(LIGHT_BLUE_TEXT("stack after check canaries:\n"));

    STACK_ASSERT(stack, file, line, func);

    return 0;
}

hash_t hash_djb2 (const char* str, size_t size_of_str)
{
    hash_t hash = 5381;

    for (size_t i = 0; i < size_of_str; i++)
    {
        hash_t a = (hash_t) *str++;
        hash = ((hash << 5) + hash) + a; // or hash * 33 + a
    }

    return hash;
}
