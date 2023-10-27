#ifndef MOVE_H
#define MOVE_H

#include <string>
#include "bitboard.h"

#define NEW_MOVE(source, dest, castle, promote, isEp) (isEp | ((promote) << 1) | ((castle) << 4) | ((dest) << 6) | ((source) << 12))
#define SOURCE(move) (((move) >> 12) & 0b111111)
#define DEST(move) (((move) >> 6) & 0b111111)
// 1 = short, 2 = long
#define CASTLE(move) (((move) >> 4) & 0b11)
#define PROMOTE(move) (((move) >> 1) & 0b111)
#define IS_EP(move) ((move) & 0b1)

namespace move {
	struct Move {
		int source;
		int dest;
		int castle;
		int promote;
		bool isEp;
	};

	std::string to_string(int move);
	int uci_to_move(const bitboard::Position &board, std::string uci);
	int uci_to_move(const bitboard::Position &board, int from, int dest, int promote);

	void make_move(bitboard::Position &board, int move);
}

#endif