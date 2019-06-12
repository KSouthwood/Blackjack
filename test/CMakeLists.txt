add_executable(testblackjack test_blackjack.c ../src/deck_of_cards.c ../src/logger.c)
add_executable(testcurses test_curses.c ../src/curses_output.c ../src/deck_of_cards.c ../src/logger.c)
target_link_libraries(testblackjack zlog pthread)
target_link_libraries(testcurses ncursesw zlog pthread)