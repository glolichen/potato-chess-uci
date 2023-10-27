#ifndef PERFT_H
#define PERFT_H

#include "bitboard.h"

namespace perft {
	struct PerftResult {
		ull totalNodes;
		int time;
	};

	PerftResult test(const bitboard::Position &board, int depth);
	void perft_atomic_first(const bitboard::Position &board, int depth);
	void perft_atomic(const bitboard::Position &board, int depth, int prevMove);
	ull perft(const bitboard::Position &board, int depth);
}

#endif
