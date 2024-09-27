#
# Simple Test Makefile
# Mike Lam, James Madison University, August 2019
#
# This version of the Makefile includes support for building a test suite. The
# recommended framework is Check (http://check.sourceforge.net/). To build and
# run the test suite, execute the "test" target. The test suite must be located
# in a module called "testsuite". The MODS, LIBS, and OBJS variables work as
# they do in the main Makefile. (For this version of the makefile, some of these
# have been moved to an external configuration file to minimize the changes
# between projects.)
#
# To change the default build target (which executes when you just type
# "make"), change the right-hand side of the definition of the "default"
# target.
#
# By default, this makefile will build the project with debugging symbols and
# without optimization. To change this, edit or remove the "-g" and "-O0"
# options in CFLAGS and LDFLAGS accordingly.
#
# By default, this makefile build the application using the GNU C compiler,
# adhering to the C11 standard with all warnings enabled.


# application-specific settings and run target

EXE=../decaf
TEST=testsuite
MODS=public.o
include make.config
LIBS=

UTESTOUT=utests.txt
ITESTOUT=itests.txt

default: $(TEST)

$(EXE):
	make -C ../

test: utest itest
	@echo "========================================"

utest: $(EXE) $(TEST)
	@echo "========================================"
	@echo "             UNIT TESTS"
	@./$(TEST) 2>/dev/null >$(UTESTOUT)
	@cat $(UTESTOUT) | sed -n -e '/Checks/,$$p' | sed -e 's/^private.*:[EF]://g'

itest: $(EXE)
	@echo "========================================"
	@echo "          INTEGRATION TESTS"
	@./integration.sh | tee $(ITESTOUT)


# compiler/linker settings

CC=gcc
CFLAGS=-g -O0 -Wall --std=c11 -pedantic
LDFLAGS=-g -O0

CFLAGS+=-I/opt/homebrew/include -Wno-gnu-zero-variadic-macro-arguments -I../include
LIBS+=-L/opt/homebrew/lib -lcheck -lm -lpthread

ifeq ($(shell uname -s),Linux)
	LIBS+=-lrt -lsubunit
endif


# build targets

$(TEST): $(TEST).o $(MODS) $(OBJS)
	$(CC) $(LDFLAGS) -o $(TEST) $^ $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -rf $(TEST) $(TEST).o $(MODS) $(UTESTOUT) $(ITESTOUT) outputs valgrind

.PHONY: default clean test unittest inttest

