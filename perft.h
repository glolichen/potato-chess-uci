#ifndef PERFT_H
#define PERFT_H

#include "bitboard.h"

namespace perft {
	struct PerftResult {
		ull totalNodes;
		int time;
	};

	PerftResult test(bitboard::Position &board, int depth);
	void perftAtomicFirst(const bitboard::Position &board, int depth);
	void perftAtomic(const bitboard::Position &board, int depth, int prevMove);
	ull perft(const bitboard::Position &board, int depth);
}

#endif
