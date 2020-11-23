.PHONY: clean
CC=clang
CFLAGS=-I. -g -Wall --std=gnu11 -fpic
SRCS=shell.c background.c commands.c background_test.c shell_test.c
OBJS=$(patsubst %.c,%.o,$(SRCS))
PROGRAM=shell

$(PROGRAM): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJS) $(PROGRAM)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)