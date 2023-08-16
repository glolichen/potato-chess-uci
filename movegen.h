#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <string>
#include <vector>
#include "move.h"

namespace movegen {
	ull get_pawn_attacks(int square, bool color);
	ull get_knight_attacks(int square);
	ull get_bishop_attacks(const ull &pieces_no_king, int square);
	ull get_rook_attacks(const ull &pieces_no_king, int square);
	ull get_queen_attacks(const ull &pieces_no_king, int square);

	ull get_checks(const bitboard::Position &board, bool color);
	void move_gen(const bitboard::Position &board, std::vector<int> &moves);
	void move_gen_with_ordering(const bitboard::Position &board, std::vector<int> &moves);
}

#endif