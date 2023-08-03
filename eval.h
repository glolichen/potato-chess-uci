#ifndef EVAL_H
#define EVAL_H

#include <vector>

namespace eval {
	void init();
	int evaluate(const bitboard::Position &board);
}

#endif
