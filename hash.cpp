#include <tuple>
#include <random>

#include "bitboard.h"
#include "hash.h"

// table #, square, piece
ull zhPieces[64][12];
// table #, castle data (0b11 = kingside and queenside rights, etc) color
ull zhCastle[4][2];

void hash::init() {
	std::random_device rd;
	std::mt19937_64 random(rd());
	std::uniform_int_distribution<ull> distribution;

	for (int j = 0; j < 64; j++) {
		for (int k = 0; k < 12; k++)
			zhPieces[j][k] = distribution(random);
	}

	for (int j = 0; j < 4; j++) {
		for (int k = 0; k < 2; k++)
			zhCastle[j][k] = distribution(random);
	}
}

ull hash::get_hash(const bitboard::Position &board) {
	ull hash = 0;

	ull pieces = board.allPieces;
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		int piece = board.mailbox[pos];
		if (piece != -1)
			hash ^= zhPieces[pos][piece];
		SET0(pieces, pos);
	}

	ull whiteCastle = (board.castle[WHITE_SHORT] << 1) | board.castle[WHITE_LONG];
	ull blackCastle = (board.castle[BLACK_SHORT] << 1) | board.castle[BLACK_LONG];

	hash ^= zhCastle[whiteCastle][WHITE];
	hash ^= zhCastle[blackCastle][BLACK];
	
	return hash;
}
