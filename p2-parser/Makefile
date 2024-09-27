#
# Simple Makefile
# Mike Lam, James Madison University, August 2019
#
# This makefile builds a simple application that contains a main module
# (specified by the EXE variable) and a predefined list of additional modules
# (specified by the MODS variable). If there are any external library
# dependencies (e.g., the math library, "-lm"), list them in the LIBS variable.
# If there are any precompiled object files, list them in the OBJS variable.
# (For this version of the makefile, some of these have been moved to an
# external configuration file to minimize the changes between projects.)
#
# By default, this makefile will build the project with debugging symbols and
# without optimization. To change this, edit or remove the "-g" and "-O0"
# options in CFLAGS and LDFLAGS accordingly.
#
# By default, this makefile build the application using the GNU C compiler,
# adhering to the C11 standard with all warnings enabled.


# application-specific settings and run target

EXE=decaf
include make.config
LIBS=

default: $(EXE)

test: $(EXE)
	make -C tests test

docs: Doxyfile
	doxygen $<

# compiler/linker settings

CC=gcc
CFLAGS=-g -O0 -Wall --std=c11 -pedantic -Iinclude
LDFLAGS=-g -O0


# build targets

$(EXE): $(MODS) $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXE) $^ $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f $(EXE) $(MODS)
	make -C tests clean

.PHONY: default clean

