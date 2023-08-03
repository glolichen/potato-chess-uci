#ifndef PERFT_H
#define PERFT_H

#include "bitboard.h"

namespace perft {
	struct PerftResult {
		ull totalNodes;
		int time;
	};

	PerftResult test(bitboard::Position &board, int depth);
	ull perft(const bitboard::Position &board, int depth, bool first);
}

#endif
