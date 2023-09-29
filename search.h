#ifndef SEARCH_H
#define SEARCH_H

#include <string>
#include "bitboard.h"
#include "move.h"

namespace search {
	struct SearchResult {
		int move;
		int depth;
		int eval;
		bool mateFound;
	};

	void pvs(int &result, const bitboard::Position &board, int depth, int alpha, int beta, int depth_from_start, bool useThreads);

	SearchResult search(bitboard::Position &board, int time_MS, int depth, bool full_search);

	int eval_is_mate(int eval);
}

#endif
