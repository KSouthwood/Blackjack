add_executable(blackjack blackjack.c deck_of_cards.c curses_output.c logger.c)
target_link_libraries(blackjack ncursesw zlog pthread)