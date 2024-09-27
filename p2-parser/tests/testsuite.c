#include "testsuite.h"

jmp_buf decaf_error;

void Error_throw_printf (const char* format, ...)
{
    longjmp(decaf_error, 1);
}

ASTNode* run_parser (char* text)
{
    if (setjmp(decaf_error) == 0) {
        /* no error */
        return parse(lex(text));
    } else {
        /* error; return NULL */
        return NULL;
    }
}

bool valid_program (char* text)
{
    return run_parser(text) != NULL;
}

bool invalid_program (char* text)
{
    return run_parser(text) == NULL;
}

extern void public_tests (Suite *s);
extern void private_tests (Suite *s);

Suite * test_suite (void)
{
    Suite *s = suite_create ("Default");
    public_tests (s);
    private_tests (s);
    return s;
}

void run_testsuite (void)
{
    Suite *s = test_suite ();
    SRunner *sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);
    srunner_free (sr);
}

int main (void)
{
    srand((unsigned)time(NULL));
    run_testsuite ();
    return EXIT_SUCCESS;
}
