#ifndef HASH_H
#define HASH_H

#include <tuple>
#include "bitboard.h"
#include "hashdefs.h"

namespace hash {
	void init();
	hashdefs::ZobristTuple hash(const bitboard::Position &board);
}

#endif
