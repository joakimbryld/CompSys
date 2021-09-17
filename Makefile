CC=gcc
CFLAGS=-std=c11 -Wall -Werror -Wextra -pedantic -g

A0 : A0.c
	$(CC) -o A0 A0.c $(CFLAGS)