/**
 * @file token.h
 * @brief Tokens and regular expressions
 *
 * The structures and functions declared in this file will likely only be
 * useful in Project 1 (lexing).
 */

#ifndef __TOKENS_H
#define __TOKENS_H

#include "common.h"

/**
 * @brief Compiled regular expression
 *
 * This is just a thin wrapper around POSIX regular expressions, so @c Regex
 * is just a typedef for @c regex_t.
 *
 * Allocate with @ref Regex_new and de-allocate with @ref Regex_free.
 *
 * Methods:
 * - @ref Regex_match
 */
typedef regex_t Regex;

/**
 * @brief Allocate and compile a new regular expression
 *
 * @param regex String containing regular expression to compile
 * @returns Newly-compiled regular expression
 */
Regex* Regex_new (const char* regex);

/**
 * @brief Match a regular expression against some text.
 *
 * If the regex matches, the matched text will be written into the given match
 * buffer.
 *
 * @param regex Compiled regular expression to match against
 * @param text  Text to match
 * @param match Character buffer (must be at least #MAX_TOKEN_LEN long)
 * @returns True if and only if the text matched the regular expression
 */
bool Regex_match (Regex *regex, const char *text, char *match);

/**
 * @brief Deallocate a regular expression
 *
 * @param regex Compiled regular expression to deallocate
 */
void Regex_free (Regex* regex);

/**
 * @brief Valid token types
 *
 * May be any of the following:
 *
 * <ul>
 * <li> @c ID - identifier </li>
 * <li> @c DECLIT - positive decimal integer literal </li>
 * <li> @c HEXLIT - positive hexadecimal integer literal (prefixed by '0x') </li>
 * <li> @c STRLIT - string literal (enclosed in quote marks) </li>
 * <li> @c KEY - keyword </li>
 * <li> @c SYM - symbol (including multi-character symbols) </li>
 * </ul>
 */
typedef enum TokenType {
    ID, DECLIT, HEXLIT, STRLIT, KEY, SYM
} TokenType;

/**
 * @brief Single token
 * 
 * Allocate with @ref Token_new and de-allocate with @ref Token_free.
 */
typedef struct Token
{
    /**
     * @brief Type of the token
     */
    TokenType type;

    /**
     * @brief Raw text of the token
     */
    char text[MAX_TOKEN_LEN];

    /**
     * @brief Source line number
     */
    int line;

    /**
     * @brief Pointer to next token (used to store in a list)
     */
    struct Token* next;

} Token;

/**
 * @brief Convert a token type to a string for output
 *
 * @param type Type to convert
 * @returns Static const string representation of the given type
 */
const char* TokenType_to_string(TokenType type);

/**
 * @brief Check string equality for tokens. Limits comparison to @c
 * MAX_TOKEN_LEN for safety.
 *
 * @param str1 First string to compare
 * @param str2 Second string to compare
 * @return True if strings are equal; false otherwise
 */
bool token_str_eq(const char* str1, const char* str2);

/**
 * @brief Allocate and initialize a new token
 *
 * Make sure Token_free() is called to deallocate the token, otherwise there
 * will be a memory leak.
 *
 * @param type Type of new token
 * @param text Raw text for new token
 * @param line Line number of new token
 * @returns Newly-created token
 */
Token* Token_new (TokenType type, const char* text, int line);

/**
 * @brief Deallocate a token
 *
 * @param token Token to deallocate
 */
void Token_free (Token* token);

/**
 * @brief Linked list of tokens
 * 
 * Allocate with @ref TokenQueue_new and de-allocate with @ref TokenQueue_free.
 * 
 * Methods:
 * - @ref TokenQueue_peek
 * - @ref TokenQueue_remove
 * - @ref TokenQueue_is_empty
 * - @ref TokenQueue_size
 * - @ref TokenQueue_print
 */
typedef struct TokenQueue
{
    /**
     * @brief Front of list (or <tt>NULL</tt> if list is empty)
     */
    Token* head;

    /**
     * @brief Back of list (or <tt>NULL</tt> if list is empty)
     */
    Token* tail;

} TokenQueue;

/**
 * @brief Allocate and initialize a new, empty queue of tokens
 *
 * @returns Newly-created queue of tokens
 */
TokenQueue* TokenQueue_new (void);

/**
 * @brief Add a token to a queue
 *
 * @param queue Queue to add to
 * @param token Token to add
 */
void TokenQueue_add (TokenQueue* queue, Token* token);

/**
 * @brief Return the next token from a queue without removing it
 * (first-in-first-out)
 *
 * @param queue Queue to look at
 * @returns Token extracted
 */
Token* TokenQueue_peek (TokenQueue* queue);

/**
 * @brief Remove a token from a queue (first-in-first-out)
 *
 * @param queue Queue to remove from
 * @returns Token removed
 */
Token* TokenQueue_remove (TokenQueue* queue);

/**
 * @brief Check whether a queue is empty
 *
 * @param queue Queue to check
 * @returns True if and only if the queue is empty
 */
bool TokenQueue_is_empty (TokenQueue* queue);

/**
 * @brief Calculate size of the queue
 *
 * @param queue Queue to check
 * @returns Number of tokens in the queue
 */
size_t TokenQueue_size (TokenQueue* queue);

/**
 * @brief Print a queue to the given file descriptor (debug output)
 *
 * @param queue Queue to print
 * @param out File stream to print to
 */
void TokenQueue_print (TokenQueue* queue, FILE* out);

/**
 * @brief Deallocate a token queue
 *
 * Also deallocates any remaining tokens
 *
 * @param queue Queue to deallocate
 */
void TokenQueue_free (TokenQueue* queue);

#endif
