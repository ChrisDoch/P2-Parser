/**
 * @file p1-lexer.h
 * @brief Interface for Project 1 (Lexer)
 */

#ifndef __P1_LEXER_H
#define __P1_LEXER_H

#include "common.h"
#include "token.h"

/**
 * @brief Convert a string containing a Decaf program into a queue of tokens.
 *
 * @param text String to lex
 * @returns Newly-created queue of tokens
 */
TokenQueue* lex(const char* text);

#endif
