#compiler to use
CC = clang

# flags to pass compiler
CFLAGS = -fsanitize=signed-integer-overflow -fsanitize=undefined -ggdb3 -O0 -Qunused-arguments -std=c11 -Wall -Werror -Wextra -Wno-sign-compare -Wshadow

# name for executable
TEST = blackjack_test

# space-separated list of header files
TEST_HDRS = deck_of_cards.h

# space-separated list of libraries, if any,
# each of which should be prefixed with -l
TEST_LIBS = 

# space-separated list of source files
TEST_SRCS = blackjack_test.c deck_of_cards.c

# automatically generated list of object files
TEST_OBJS = $(TEST_SRCS:.c=.o)

all:	$(TEST)

$(TEST): $(TEST_OBJS) $(TEST_HDRS) Makefile
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(LIBS)

# dependencies
$(TEST_OBJS): $(TEST_HDRS) Makefile

# housekeeping
clean:
	rm -f core $(TEST) *.o
