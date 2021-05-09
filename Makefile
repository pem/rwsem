#
# pem 2021-05-08
#
#

CC=gcc -std=c99

CCOPTS=-pedantic -Wall -Werror

CCDEFS=-D_XOPEN_SOURCE=500

CFLAGS=-g -DDEBUG $(CCOPTS) $(CCDEFS)
#CFLAGS=-O $(CCOPTS) $(CCDEFS)
LDFLAGS=
LDLIBS=-lpthread -lrt

PROG=rwsem-test

LIB=librwsem.a

SRC=rwsem.c

OBJ=$(SRC:%.c=%.o)

all:	$(PROG)

$(PROG):	rwsem-test.o $(LIB)

$(LIB):	$(COBJ) $(OBJ)
	rm -f $(LIB)
	$(AR) qc $(LIB) $(COBJ) $(OBJ)
	ranlib $(LIB)

clean:
	$(RM) rwsem-test.o $(OBJ) core

cleanall:	clean
	$(RM) $(PROG) $(LIB) make.deps

make.deps:
	gcc -MM $(CFLAGS) $(SRC) > make.deps

include make.deps
