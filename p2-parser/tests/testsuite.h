/**
 * @file testsuite.h
 * @brief Testing utility functions
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#include <check.h>

#include "p1-lexer.h"
#include "p2-parser.h"

/**
 * @brief Define a test case with a valid program
 */
#define TEST_VALID(NAME,TEXT) START_TEST (NAME) \
{ ck_assert (valid_program(TEXT)); } \
END_TEST

/**
 * @brief Define a test case with an invalid program
 */
#define TEST_INVALID(NAME,TEXT) START_TEST (NAME) \
{ ck_assert (invalid_program(TEXT)); } \
END_TEST

/**
 * @brief Define a test case with a valid main function
 */
#define TEST_VALID_MAIN(NAME,TEXT) START_TEST (NAME) \
{ ck_assert (valid_program("def int main () { " TEXT " }")); } \
END_TEST

/**
 * @brief Define a test case with an invalid main function
 */
#define TEST_INVALID_MAIN(NAME,TEXT) START_TEST (NAME) \
{ ck_assert (invalid_program("def int main () { " TEXT " }")); } \
END_TEST

/**
 * @brief Define a test case with a valid main function returning an expression
 */
#define TEST_VALID_EXPR(NAME,TEXT) START_TEST (NAME) \
{ ck_assert (valid_program("def int main () { return " TEXT " ; }")); } \
END_TEST

/**
 * @brief Define a test case with an invalid main function returning an expression
 */
#define TEST_INVALID_EXPR(NAME,TEXT) START_TEST (NAME) \
{ ck_assert (invalid_program("def int main () { return " TEXT " ; }")); } \
END_TEST

/**
 * @brief Define a test case for an integer literal
 */
#define TEST_INT_LITERAL(NAME,TEXT,VALUE) START_TEST (NAME) \
{ ASTNode* p = run_parser("def int main() { return " TEXT " ; }"); \
  int value = p->program.functions->head->funcdecl.body->block.statements->head->funcreturn.value->literal.integer; \
  ck_assert_int_eq(value, VALUE); } \
END_TEST

/**
 * @brief Define a test case for a string literal
 */
#define TEST_STR_LITERAL(NAME,TEXT,VALUE) START_TEST (NAME) \
{ ASTNode* p = run_parser("def int main() { return " TEXT " ; }"); \
  char* value = p->program.functions->head->funcdecl.body->block.statements->head->funcreturn.value->literal.string; \
  ck_assert_str_eq(value, VALUE); } \
END_TEST

/**
 * @brief Add a test to the test suite
 */
#define TEST(NAME) tcase_add_test (tc, NAME)

/**
 * @brief Run lexer and parser on given text
 *
 * The difference between this and the main parse() function is that this version
 * catches exceptions and returns @c NULL instead of propogating the exception.
 *
 * @param text Code to lex and parse
 * @returns AST or @c NULL if there was an error
 */
ASTNode* run_parser (char* text);

/**
 * @brief Run lexer and parser on given text and verify that it throws an exception.
 *
 * @param text Code to lex and parse
 * @returns True if and only if the lexer or parser threw an exception
 */
bool invalid_program (char* text);

/**
 * @brief Run lexer and parser on given text and verify that it returns an AST and
 * does not throw an exception.
 *
 * @param text Code to lex and parse
 * @returns True if and only if the text was lexed and parsed successfully
 */
bool valid_program (char* text);
