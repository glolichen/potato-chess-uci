#ifndef BITBOARD_H
#define BITBOARD_H

#include <string>
#include <vector>
#include <map>

#include "hashdefs.h"

#define SET1(bitboard, pos) (bitboard |= 1ull << (pos))
#define SET0(bitboard, pos) (bitboard ^= 1ull << (pos))
#define QUERY(bitboard, pos) (bitboard >> (pos) & 1)

using ull = unsigned long long;
enum Color { WHITE, BLACK };
enum Pieces { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, ALL };
enum Castle { WHITE_SHORT, WHITE_LONG, BLACK_SHORT, BLACK_LONG };
enum Squares {
	H1, G1, F1, E1, D1, C1, B1, A1,
	H2, G2, F2, E2, D2, C2, B2, A2,
	H3, G3, F3, E3, D3, C3, B3, A3,
	H4, G4, F4, E4, D4, C4, B4, A4,
	H5, G5, F5, E5, D5, C5, B5, A5,
	H6, G6, F6, E6, D6, C6, B6, A6,
	H7, G7, F7, E7, D7, C7, B7, A7,
	H8, G8, F8, E8, D8, C8, B8, A8,
};

namespace bitboard {
	struct Position {
		ull all_pieces;
		ull pieces[2][7];
		int mailbox[64];

		bool turn;
		int en_passant;
		bool castle[4];

		int fifty_move_clock;
		int full_move;
	};

	extern Position board;
	
	extern std::string squares[];
	extern std::string pieces;

	std::vector<std::string> split(std::string str, char split_on);

	void decode(std::string fen);
	std::string encode(const Position &board);

	void print_board(const Position &board);
}

#endif
