#ifndef BOOK_H
#define BOOK_H

#include <string>
#include "bitboard.h"

namespace book {
	ull gen_polyglot_key(const bitboard::Position &board);

	void book_open(const char fileName[]);
	int book_move(const bitboard::Position &board);
}

#endif
