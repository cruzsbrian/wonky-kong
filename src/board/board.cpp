#include "board.h"

#include <cstdlib>


using namespace std;


uint16_t Board::hash_vals[2][64];

void Board::init_hash() {
    srand(1337);

    for (auto i = 0; i < 64; ++i) {
        Board::hash_vals[Color::BLACK][i] = rand();
        Board::hash_vals[Color::WHITE][i] = rand();
    }
}




/* ====== MOVE GENERATION AND MAKE MOVE ====== */

/* Masks to filter out end files. */
const uint64_t notAFile = 0xfefefefefefefefe;
const uint64_t notHFile = 0x7f7f7f7f7f7f7f7f;

/*
 * Fill algorithms:
 * Smears the gen bitmap in the specified direction as long as there is
 * something in the pro bitmap. For east, west, and diagonals, we exclude the
 * first file in that shift direction to avoid wrap-around.
 *
 * Shift algorithms:
 * Shifts the pieces one step in the specified direction, making sure pieces
 * don't wrap around.
 *
 * The result takes each piece in gen, and draws a ray from it in the specified
 * direction as far as pieces in pro go in that direction.
 */
uint64_t soutOccl(uint64_t gen, uint64_t pro) {
    gen |= pro & (gen >> 8);
    pro &=       (pro >> 8);
    gen |= pro & (gen >> 16);
    pro &=       (pro >> 16);
    gen |= pro & (gen >> 32);
    return gen;
}

uint64_t soutOne(uint64_t gen) {
    return gen >> 8;
}

uint64_t nortOccl(uint64_t gen, uint64_t pro) {
    gen |= pro & (gen << 8);
    pro &=       (pro << 8);
    gen |= pro & (gen << 16);
    pro &=       (pro << 16);
    gen |= pro & (gen << 32);
    return gen;
}

uint64_t nortOne(uint64_t gen) {
    return gen << 8;
}

uint64_t eastOccl(uint64_t gen, uint64_t pro) {
    pro &= notAFile;
    gen |= pro & (gen << 1);
    pro &=       (pro << 1);
    gen |= pro & (gen << 2);
    pro &=       (pro << 2);
    gen |= pro & (gen << 4);
    return gen;
}

uint64_t eastOne(uint64_t gen) {
    return (gen & notAFile) << 1;
}

uint64_t westOccl(uint64_t gen, uint64_t pro) {
    pro &= notHFile;
    gen |= pro & (gen >> 1);
    pro &=       (pro >> 1);
    gen |= pro & (gen >> 2);
    pro &=       (pro >> 2);
    gen |= pro & (gen >> 4);
    return gen;
}

uint64_t westOne(uint64_t gen) {
    return (gen & notHFile) >> 1;
}

uint64_t noEaOccl(uint64_t gen, uint64_t pro) {
    pro &= notAFile;
    gen |= pro & (gen << 9);
    pro &=       (pro << 9);
    gen |= pro & (gen << 18);
    pro &=       (pro << 18);
    gen |= pro & (gen << 36);
    return gen;
}

uint64_t noEaOne(uint64_t gen) {
    return (gen & notAFile) << 9;
}

uint64_t soEaOccl(uint64_t gen, uint64_t pro) {
    pro &= notAFile;
    gen |= pro & (gen >> 7);
    pro &=       (pro >> 7);
    gen |= pro & (gen >> 14);
    pro &=       (pro >> 14);
    gen |= pro & (gen >> 28);
    return gen;
}

uint64_t soEaOne(uint64_t gen) {
    return (gen & notAFile) >> 7;
}

uint64_t noWeOccl(uint64_t gen, uint64_t pro) {
    pro &= notHFile;
    gen |= pro & (gen << 7);
    pro &=       (pro << 7);
    gen |= pro & (gen << 14);
    pro &=       (pro << 14);
    gen |= pro & (gen << 28);
    return gen;
}

uint64_t noWeOne(uint64_t gen) {
    return (gen & notHFile) << 7;
}

uint64_t soWeOccl(uint64_t gen, uint64_t pro) {
    pro &= notHFile;
    gen |= pro & (gen >> 9);
    pro &=       (pro >> 9);
    gen |= pro & (gen >> 18);
    pro &=       (pro >> 18);
    gen |= pro & (gen >> 36);
    return gen;
}

uint64_t soWeOne(uint64_t gen) {
    return (gen & notHFile) >> 9;
}


/*
 * Move generation:
 * Use own pieces as generator, opponents pieces as propogator. After rays have
 * been calculated, they must be &-ed with pro to exclude the generator piece.
 * Then shift one more step in the ray direction (making sure not to wrap
 * around), and & with empty squares to get playable squares.
 *
 * Returns a long representing squares that can be played in.
 */
uint64_t Board::get_moves(Color c) const {
    uint64_t gen, pro, empty, tmp, moves;

    if (c == Color::BLACK) {
        gen = b;
        pro = w;
    } else {
        gen = w;
        pro = b;
    }

    moves = 0L;
    empty = ~(gen | pro);

    tmp = soutOccl(gen, pro) & pro;
    moves |= (tmp >> 8) & empty;

    tmp = nortOccl(gen, pro) & pro;
    moves |= (tmp << 8) & empty;

    tmp = eastOccl(gen, pro) & pro;
    moves |= (tmp << 1) & notAFile & empty;

    tmp = westOccl(gen, pro) & pro;
    moves |= (tmp >> 1) & notHFile & empty;

    tmp = noEaOccl(gen, pro) & pro;
    moves |= (tmp << 9) & notAFile & empty;

    tmp = soEaOccl(gen, pro) & pro;
    moves |= (tmp >> 7) & notAFile & empty;

    tmp = noWeOccl(gen, pro) & pro;
    moves |= (tmp << 7) & notHFile & empty;

    tmp = soWeOccl(gen, pro) & pro;
    moves |= (tmp >> 9) & notHFile & empty;

    return moves;
}


/**
 * Get frontier pieces:
 * Gives the number of pieces of color c that are adjacent to empty squares.
 * Takes the pieces of color c, and shifts in every direction, &-ing with empty
 * pieces.
 */
int Board::get_frontier(Color c) const {
    uint64_t own, opp, empty, frontier;

    if (c == Color::BLACK) {
        own = b;
        opp = w;
    } else {
        own = w;
        opp = b;
    }

    empty = ~(own | opp);
    frontier = 0L;

    frontier |= (own >> 8) & empty;
    frontier |= (own << 8) & empty;
    frontier |= (own << 1) & notAFile & empty;
    frontier |= (own >> 1) & notHFile & empty;
    frontier |= (own << 9) & notAFile & empty;
    frontier |= (own >> 7) & notAFile & empty;
    frontier |= (own << 7) & notHFile & empty;
    frontier |= (own >> 9) & notHFile & empty;

    return popcount(frontier);
}


/*
 * Make-move: Makes a move for color c in position pos, and updates the board's
 * hash. Give -1 as pos for pass.  Gen is a one-hot long representing the added
 * piece. Filling from gen along opponent pieces and &-ing with rays in the
 * opposite direction from existing pieces gives only lines of opponent pieces
 * that have the new piece on one side and an existing own piece on the other
 * side.
 */
void Board::do_move(int pos, Color c) {
    uint64_t gen, own, pro, diff;

    /* -1 for pass. */
    if (pos == -1) {
        return;
    }

    gen = 1L << pos;

    if (c == Color::BLACK) {
        own = b;
        pro = w;

        b |= gen;
    } else {
        own = w;
        pro = b;

        w |= gen;
    }

    diff = 0L;
    diff |= soutOccl(gen, pro) & nortOccl(own, pro);
    diff |= nortOccl(gen, pro) & soutOccl(own, pro);
    diff |= eastOccl(gen, pro) & westOccl(own, pro);
    diff |= westOccl(gen, pro) & eastOccl(own, pro);
    diff |= noEaOccl(gen, pro) & soWeOccl(own, pro);
    diff |= soEaOccl(gen, pro) & noWeOccl(own, pro);
    diff |= noWeOccl(gen, pro) & soEaOccl(own, pro);
    diff |= soWeOccl(gen, pro) & noEaOccl(own, pro);

    b ^= diff;
    w ^= diff;
    hash ^= hash_vals[c][pos];
}


void Board::add_piece(int pos, Color c) {
    if (c == Color::BLACK) {
        b |= (1L << pos);
    } else {
        w |= (1L << pos);
    }

    hash ^= hash_vals[c][pos];
}




string Board::to_str() const {
    string ret = "  a b c d e f g h\n";

    for (auto i = 0; i < 8; ++i) {
        ret += to_string(i + 1) + " ";
        for (auto j = 0; j < 8; ++j) {
            int pos = (i * 8) + j;

            bool black = (b >> pos) & 1L;
            bool white = (w >> pos) & 1L;

            if (white) {
                ret += "O ";
            } else if (black) {
                ret += "# ";
            } else {
                ret += "_ ";
            }
        }
        ret += "\n";
    }

    return ret;
}




string mask_to_str(uint64_t x) {
    string ret = "  a b c d e f g h\n";

    for (auto i = 0; i < 64; ++i) {
        if (i % 8 == 0) ret += to_string((i / 8) + 1) + " ";
        ret += ((x >> i) & 1L) ? "X " : "_ ";
        if (i % 8 == 7)  ret += "\n"; 
    }
    ret += "\n";

    return ret;
}


int popcount(uint64_t x) {
    const uint64_t m1  = 0x5555555555555555;
    const uint64_t m2  = 0x3333333333333333;
    const uint64_t m4  = 0x0f0f0f0f0f0f0f0f;
    const uint64_t h01 = 0x0101010101010101;

    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    return (x * h01) >> 56;
}
