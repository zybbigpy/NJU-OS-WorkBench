NAME := $(shell basename $(PWD))
export TASK := M6
all: $(NAME)-64.so $(NAME)-32.so
CFLAGS += -U_FORTIFY_SOURCE

include ../Makefile
include ../Makefile.lab

test:
	gcc -m64 -g -O1 -Wall -o test test.c ./libkvdb-64.so && ./test

testclean:
	rm *.db *.log ./test