/**
 * @file common.h
 * @brief Includes, constants, declarations, and macros used across the compiler
 *
 * This module declares a list of standard C library dependencies used by most
 * modules, a list of constants used across the compiler, a couple of
 * utility functions, and several useful macros.
 */

#ifndef __COMMON_H
#define __COMMON_H

/** \mainpage
 * 
 * Shortcuts for Doxygen-based documentation:
 *   * [Structs](annotated.html)
 *   * [Files](files.html)
 */

/*
 * DEPENDENCIES
 */

#include <inttypes.h>
#include <regex.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Maximum size (in bytes) of any Decaf source file
 */
#define MAX_FILE_SIZE 65536

/**
 * @brief Maximum length (in characters) of any single line of input
 */
#define MAX_LINE_LEN    256

/**
 * @brief Maximum length (in characters) of any single token
 */
#define MAX_TOKEN_LEN   256

/**
 * @brief Maximum length (in characters) of any error message
 */
#define MAX_ERROR_LEN   256

/**
 * @brief Maximum length (in characters) of any identifier
 */
#define MAX_ID_LEN      256

/**
 * @brief Valid Decaf types
 *
 * Variables can only be @c INT or  @c BOOL; the others are included for
 * tracking the return type of a @c void function or the type of a parameter to
 * @c print_str.
 */
typedef enum DecafType{
    UNKNOWN, INT, BOOL, VOID, STR
} DecafType;

/**
 * @brief Convert a Decaf type to a string for output
 *
 * @param type Type to convert
 * @returns Static const string representation of the given type
 */
const char* DecafType_to_string(DecafType type);

/**
 * @brief Print a Decaf string literal, inserting escape codes as necessary.
 * 
 * @param string String literal to print
 * @param output File stream to print the string to
 */
void print_escaped_string(const char* string, FILE* output);

/**
 * @brief Print a Decaf string literal, inserting double escape codes as necessary.
 * 
 * This is primarily useful for AST output w/ string literals and/or ILOC code.
 * 
 * @param string String literal to print
 * @param output File stream to print the string to
 */
void print_doubly_escaped_string(const char* string, FILE* output);

/**
 * @brief Throw an exception with an error message using @c printf syntax
 *
 * This function uses the @c longjmp functionality in the standard C library to
 * implement exception handling for the lexing and parsing phases. The code
 * that might throw an exception must be wrapped in a @c setjmp block.
 *
 * Because this function must have access to the @c jmp_buf used to connect the
 * @c setjmp callsite with the @c longjmp, this function must be implemented in
 * the compiler or test driver (where the @c setjmp call will be) in other to
 * avoid having an @c extern static variable. Thus, there is no implementation
 * in @c common.c, and it is only declared here so that it can be called in the
 * various front end phases of compilation.
 */
void Error_throw_printf (const char* format, ...);

/**
 * @brief Check a pointer for NULL and terminate with an out-of-memory error
 * 
 * This is meant to be called immediately after a malloc/calloc call.
 */
#define CHECK_MALLOC_PTR(P) \
    if (P == NULL) { \
        printf("Out of memory!\n"); \
        exit(EXIT_FAILURE); \
    }

/**
 * @brief Declare a singly-linked list structure of the given type
 * 
 * This avoids having to declare a separate structure for every type that we
 * need to be able to store in lists.
 * 
 * @param NAME Prefix for the list struct name (actual name will be @c NAMEList)
 * @param ELEMTYPE Type of the elements to be stored (usually a struct pointer)
 */
#define DECL_LIST_TYPE(NAME, ELEMTYPE) \
    /** @brief Linked list of ELEMTYPE elements */  \
    typedef struct NAME ## List { \
        ELEMTYPE head; /**< @brief First element in list (or @c NULL if empty) */ \
        ELEMTYPE tail; /**< @brief Last element in list (or @c NULL if empty) */ \
        int size;      /**< @brief Number of elements in list */ \
    } NAME ## List; \
    \
    /** @brief Allocate and initialize a new, empty list. */ \
    NAME ## List* NAME ## List_new (void); \
    \
    /** @brief Add an item to the end of a list. */ \
    void NAME ## List_add (NAME ## List* list, ELEMTYPE item); \
    \
    /** @brief Look up the size of a list. */ \
    int NAME ## List_size (NAME ## List* list); \
    \
    /** @brief Test a list to see if it is empty. */ \
    bool NAME ## List_is_empty (NAME ## List* list); \
    \
    /** @brief Deallocate a list and any contained items. */ \
    void NAME ## List_free (NAME ## List* list);

/**
 * @brief Define a list implementation
 *
 * This avoids having to implement a separate structure for every type that we
 * need to be able to store in lists.
 * 
 * Requirement: @c ELEMTYPE must be a pointer to a struct with a self-referencing
 * @c next pointer.
 *
 * @param NAME Prefix for the list struct name (actual name will be @c NAMEList)
 * @param ELEMTYPE Type of the elements to be stored (must be a struct pointer)
 * @param FREEFUNC Name of the function to call to deallocate each element
 */
#define DEF_LIST_IMPL(NAME, ELEMTYPE, FREEFUNC) \
    NAME ## List* NAME ## List_new (void) \
    { \
        NAME ## List* list = (NAME ## List*)calloc(1, sizeof(NAME ## List)); \
        CHECK_MALLOC_PTR(list); \
        list->head = NULL; \
        list->tail = NULL; \
        list->size = 0; \
        return list; \
    } \
    void NAME ## List_add (NAME ## List* list, ELEMTYPE item) \
    { \
        if (list->head == NULL) { \
            list->head = item; \
            list->tail = item; \
        } else { \
            list->tail->next = item; \
            list->tail = item; \
        } \
        list->size++; \
    } \
    int NAME ## List_size (NAME ## List* list) \
    { \
        return list->size; \
    } \
    bool NAME ## List_is_empty (NAME ## List* list) \
    { \
        return (list->size == 0); \
    } \
    void NAME ## List_free (NAME ## List* list) \
    { \
        ELEMTYPE next = list->head; \
        while (next != NULL) { \
            ELEMTYPE cur = next; \
            next = cur->next; \
            FREEFUNC(cur); \
        } \
        free(list); \
    }

/**
 * @brief Set up a for-each style loop over a singly-linked list
 * 
 * Works for all structures declared and implemented with @ref DECL_LIST_TYPE
 * and @ref DEF_LIST_IMPL.
 */
#define FOR_EACH(TYPE, VARIABLE, CONTAINER) \
    for (TYPE VARIABLE = (CONTAINER)->head; \
         VARIABLE != NULL; \
         VARIABLE = VARIABLE->next)

#endif
