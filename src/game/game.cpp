#include "game.h"

#include "../board/board.h"
#include "../util.h"

#include <iostream>
#include <string>


using namespace std;


Game::Game(Player *black, Player *white) {
    players[BLACK] = black;
    players[WHITE] = white;

    b = board::add_piece(b, 27, WHITE);
    b = board::add_piece(b, 28, BLACK);
    b = board::add_piece(b, 35, BLACK);
    b = board::add_piece(b, 36, WHITE);
}


const string turns[2] = {"BLACK", "WHITE"};

int Game::play(bool display) {
    bool turn = BLACK;
    bool prev_pass = false;

    while (true) {
        if (display)
            cout << board::to_str(b) << "\n";

        Player *p = players[turn];

        int move = p->next_move(b, -1);
        if (move == -1) {
            if (prev_pass) break;
            else prev_pass = true;
        } else {
            prev_pass = false;
        }

        if (display)
            cout << "\n" << turns[turn] << ": " << move_to_notation(move) << "\n";

        b = board::do_move(b, move);

        turn = !turn;
    }

    //TODO might be flipped
    int black_score = board::popcount(b.own);
    int white_score = board::popcount(b.opp);

    string score_msg;
    if (black_score > white_score) {
        score_msg = "Black wins (" + to_string(black_score) + "-" + to_string(white_score) + ")";
    } else if (white_score > black_score) {
        score_msg = "White wins (" + to_string(white_score) + "-" + to_string(black_score) + ")";
    } else {
        score_msg = "Tie";
    }

    if (display)
        cout << score_msg << "\n";

    return (black_score - white_score);
}
