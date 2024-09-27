/**
 * @file public.c
 * @brief Public test cases (and location for new tests)
 * 
 * This file provides a few basic sanity test cases and a location to add new tests.
 */

#include "testsuite.h"

#ifndef SKIP_IN_DOXYGEN

/*
 * test empty program (still valid at this point)
 */
START_TEST(D_empty)
{
    ASTNode* ast = run_parser("");
    ck_assert_ptr_ne(ast, NULL);
    ck_assert(ast->type == PROGRAM);
    ck_assert_int_eq(ast->program.variables->size, 0);
    ck_assert_int_eq(ast->program.functions->size, 0);
}
END_TEST

/*
 * test a trivial program with a couple of variables
 */
START_TEST(D_trivial)
{
    ASTNode* ast = run_parser("int a; int b;");
    ck_assert_ptr_ne(ast, NULL);
    ck_assert_int_eq(ast->program.variables->size, 2);
    ck_assert(ast->program.variables->head->type == VARDECL);
    ck_assert(ast->program.variables->head->next->type == VARDECL);
    ck_assert_int_eq(ast->program.functions->size, 0);
}
END_TEST

/*
 * Test a variety of valid programs to make sure the parser doesn't throw
 * an exception
 */

TEST_VALID(D_int_var, "int a;")
TEST_VALID(D_bool_var, "bool b;")
TEST_VALID(C_void_func, "def void foo() { }")
TEST_VALID_MAIN(C_assign, "int a; a = 5;")
TEST_VALID_MAIN(C_break, "break;")
TEST_VALID_MAIN(C_continue, "continue;")
TEST_VALID_MAIN(C_return, "return;")
TEST_VALID_MAIN(C_return_val, "return 0;")
TEST_VALID(B_param_func, "def int foo(int a, bool b) { }")
TEST_VALID_MAIN(B_conditional, "if (true) { }")
TEST_VALID_MAIN(B_whileloop, "while (false) { }")
TEST_VALID_EXPR(B_add_expr, "3+7")
TEST_VALID_EXPR(B_add_expr_bool, "true && false")
TEST_VALID_EXPR(B_neg_expr, "-2")
TEST_VALID(A_arrays, "int a[5]; def int main() { a[1] = 7; return a[1]; }")

/*
 * Test a variety of invalid programs to make sure the parser throws an
 * exception.
 */

TEST_INVALID(D_invalid_vardecl_no_type, "a")
TEST_INVALID(D_invalid_vardecl_no_semicolon, "int a")
TEST_INVALID(D_invalid_parens, "()")
TEST_INVALID(D_invalid_brackets, "[]")
TEST_INVALID(D_invalid_braces, "{}")
TEST_INVALID(D_invalid_broken_assign, "b=")
TEST_INVALID_MAIN(C_invalid_return_break, "return break;")
TEST_INVALID_EXPR(B_invalid_add, "3++8")

/*
 * Test some integer and string literals for proper handling.
 */

TEST_INT_LITERAL(C_declit, "42", 42)
TEST_INT_LITERAL(C_hexlit, "0x10", 16)
TEST_STR_LITERAL(C_strlit, "\"abc\"", "abc")
TEST_STR_LITERAL(A_newline, "\"ab\\nc\"", "ab\nc")

#endif

/**
 * @brief Register all test cases
 * 
 * @param s Test suite to which the tests should be added
 */
void public_tests (Suite *s)
{
    TCase *tc = tcase_create ("Public");

    TEST(D_empty);
    TEST(D_trivial);
    TEST(D_int_var);
    TEST(D_bool_var);

    TEST(D_invalid_vardecl_no_type);
    TEST(D_invalid_vardecl_no_semicolon);
    TEST(D_invalid_parens);
    TEST(D_invalid_brackets);
    TEST(D_invalid_braces);
    TEST(D_invalid_broken_assign);

    TEST(C_void_func);
    TEST(C_assign);
    TEST(C_break);
    TEST(C_continue);
    TEST(C_return);
    TEST(C_return_val);
    TEST(C_invalid_return_break);
    TEST(C_declit);
    TEST(C_hexlit);
    TEST(C_strlit);

    TEST(B_param_func);
    TEST(B_conditional);
    TEST(B_whileloop);
    TEST(B_add_expr);
    TEST(B_add_expr_bool);
    TEST(B_neg_expr);
    TEST(B_invalid_add);

    TEST(A_arrays);
    TEST(A_newline);

    suite_add_tcase (s, tc);
}

