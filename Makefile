blackjack:
	cd src/ && $(MAKE)

test_blackjack:
	cd test/ && $(MAKE) test_blackjack

test_curses:
	cd test/ && $(MAKE) test_curses
	
clean:
	cd src/ && $(MAKE) clean
	cd test/ && $(MAKE) clean
	
all: blackjack test_blackjack test_curses
