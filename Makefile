blackjack:
	cd src/ && $(MAKE)

blackjack_test:
	cd test/ && $(MAKE)
	
all: blackjack blackjack_test
