#ifndef HASH_H
#define HASH_H

#include "bitboard.h"

namespace hash {
	void init();
	ull get_hash(const bitboard::Position &board);
}

#endif
