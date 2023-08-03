#ifndef BOOK_H
#define BOOK_H

#include <string>
#include "bitboard.h"

namespace book {
	ull gen_polyglot_key(bitboard::Position &board);

	void book_open(const char file_name[]);
	int book_move(bitboard::Position &board);
}

#endif
