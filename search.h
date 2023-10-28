#ifndef SEARCH_H
#define SEARCH_H

#include <string>
#include "bitboard.h"
#include "move.h"

namespace search {
	struct SearchResult {
		int move, ponder, depth, eval;
	};

	void table_clear();
	void table_clear_move();

	void pvs(int &result, const bitboard::Position &board, int depth, int alpha, int beta, int last_move, int depth_from_start, bool use_threads);

	SearchResult search_by_time(const bitboard::Position &board, int time_MS, bool full_search);
	SearchResult search_by_depth(const bitboard::Position &board, int depth);
	SearchResult search_unlimited(const bitboard::Position &board);

	// time_MS = the time to keep searching after opponent makes move
	SearchResult ponder(const bitboard::Position &board, int time_MS);
	
	int eval_is_mate(int eval);
}

#endif
