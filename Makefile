#compiler to use
CC = clang

# flags to pass compiler
CFLAGS = -fsanitize=signed-integer-overflow -fsanitize=undefined -ggdb3 -O0 -Qunused-arguments -std=c11 -Wall -Werror -Wextra -Wno-sign-compare -Wshadow

# name for executable
TEST = blackjack_test
MAIN = blackjack
EXES = $(TEST) $(MAIN)

# space-separated list of header files
HDRS = deck_of_cards.h curses_output.h logger.h blackjack.h unicode_box_chars.h
TEST_HDRS = $(HDRS)
MAIN_HDRS = $(HDRS)

# space-separated list of libraries, if any,
# each of which should be prefixed with -l
LIBS = -lncursesw -lzlog -lpthread
TEST_LIBS = $(LIBS)
MAIN_LIBS = $(LIBS)

# space-separated list of source files
SRCS = deck_of_cards.c curses_output.c logger.c
TEST_SRCS = blackjack_test.c $(SRCS)
MAIN_SRCS = blackjack.c $(SRCS)

# automatically generated list of object files
TEST_OBJS = $(TEST_SRCS:.c=.o)
MAIN_OBJS = $(MAIN_SRCS:.c=.o)

all:	$(EXES)

$(TEST): $(TEST_OBJS) $(TEST_HDRS) Makefile
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(TEST_LIBS)

$(MAIN): $(MAIN_OBJS) $(MAIN_HDRS) Makefile
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJS) $(MAIN_LIBS)
	
# dependencies
$(TEST_OBJS): $(TEST_HDRS) Makefile
$(MAIN_OBJS): $(MAIN_HDRS) Makefile

# housekeeping
clean:
	rm -f core $(EXES) *.o *.log
