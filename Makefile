CFLAGS += -O3 -g3

SRCS := cli_options.c thread.c main.c
OBJS := $(SRCS:.c=.o)

PROG := test-fs

.PHONY: all clean
all: $(PROG)
$(PROG): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $^
	
clean:
	rm -f $(PROG) $(OBJS)
