blackjack:
	cd src/ && $(MAKE)

test_blackjack:
	cd test/ && $(MAKE) test_blackjack

test_curses:
	cd test/ && $(MAKE) test_curses
	
all: blackjack test_blackjack test_curses
