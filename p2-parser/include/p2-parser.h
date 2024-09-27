/**
 * @file p2-parser.h
 * @brief Interface for Project 2 (Parser)
 */

#ifndef __P2_PARSER_H
#define __P2_PARSER_H

#include "common.h"
#include "token.h"
#include "ast.h"
#include "visitor.h"

/**
 * @brief Convert a queue of tokens into an abstract syntax tree (AST)
 *
 * @param input Tokens to parse
 * @returns Root of abstract syntax tree
 */
ASTNode* parse (TokenQueue* input);

#endif
