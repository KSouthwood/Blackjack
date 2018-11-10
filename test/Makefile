#compiler to use
CC = clang

# flags to pass compiler
CFLAGS = -fsanitize=signed-integer-overflow -fsanitize=undefined -ggdb3 -O0 -Qunused-arguments -std=c11 -Wall -Werror -Wextra -Wno-sign-compare -Wshadow

# name for executable
TEST = test_blackjack
CURSES = test_curses
EXES = $(TEST) $(CURSES)

# space-separated list of header files
HDRS = ../src/deck_of_cards.h ../src/curses_output.h ../src/logger.h ../src/blackjack.h ../src/unicode_box_chars.h
TEST_HDRS = $(HDRS)
CURSES_HDRS = $(HDRS)

# space-separated list of libraries, if any,
# each of which should be prefixed with -l
LIBS = -lncursesw -lzlog -lpthread
TEST_LIBS = $(LIBS)

# space-separated list of source files
SRCS = ../src/deck_of_cards.c ../src/curses_output.c ../src/logger.c
TEST_SRCS = test_blackjack.c $(SRCS)
CURSES_SRCS = test_curses.c $(SRCS)

# automatically generated list of object files
TEST_OBJS = $(TEST_SRCS:.c=.o)
CURSES_OBJS = $(CURSES_SRCS:.c=.o)

all:	$(EXES)

$(TEST): $(TEST_OBJS) $(TEST_HDRS) Makefile
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(TEST_LIBS)
	
$(CURSES): $(CURSES_OBJS) $(HDRS) Makefile
	$(CC) $(CFLAGS) -o $@ $(CURSES_OBJS) $(LIBS)
	
# dependencies
$(TEST_OBJS): $(TEST_HDRS) Makefile
$(CURSES_OBJS): $(HDRS) Makefile

# housekeeping
clean:
	rm -f core $(EXES) *.o
