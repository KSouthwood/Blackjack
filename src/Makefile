#compiler to use
CC = clang

# flags to pass compiler
CFLAGS = -fsanitize=signed-integer-overflow -fsanitize=undefined -ggdb3 -O0 -Qunused-arguments -std=c11 -Wall -Werror -Wextra -Wno-sign-compare -Wshadow

# name for executable
MAIN = blackjack
EXES = $(MAIN)

# space-separated list of header files
HDRS = deck_of_cards.h curses_output.h logger.h blackjack.h unicode_box_chars.h
MAIN_HDRS = $(HDRS)

# space-separated list of libraries, if any,
# each of which should be prefixed with -l
LIBS = -lncursesw -lzlog -lpthread
MAIN_LIBS = $(LIBS)

# space-separated list of source files
SRCS = deck_of_cards.c curses_output.c logger.c
MAIN_SRCS = blackjack.c $(SRCS)

# automatically generated list of object files
MAIN_OBJS = $(MAIN_SRCS:.c=.o)

all:	$(EXES)

$(MAIN): $(MAIN_OBJS) $(MAIN_HDRS) Makefile
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJS) $(MAIN_LIBS)
	
# dependencies
$(MAIN_OBJS): $(MAIN_HDRS) Makefile

# housekeeping
clean:
	rm -f core $(EXES) *.o
