#include <tuple>
#include <random>

#include "bitboard.h"
#include "hash.h"

// table #, square, piece
ull zh_pieces[3][64][12];
// table #, castle data (0b11 = kingside and queenside rights, etc) color
ull zh_castle[3][4][2];

void hash::init() {
	for (int i = 0; i < 3; i++) {
		std::random_device rd;
		std::mt19937_64 random(rd());
		std::uniform_int_distribution<ull> distribution;

		for (int j = 0; j < 64; j++) {
			for (int k = 0; k < 12; k++)
				zh_pieces[i][j][k] = distribution(random);
		}

		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 2; k++)
				zh_castle[i][j][k] = distribution(random);
		}
	}
}

std::tuple<ull, ull, ull> hash::hash(const bitboard::Position &board) {
	ull hashes[3] = { 0, 0, 0 };

	for (int i = 0; i < 3; i++) {
		ull pieces = board.all_pieces;
		while (pieces) {
			int pos = __builtin_ctzll(pieces);
			int piece = board.mailbox[pos];
			if (piece != -1)
				hashes[i] ^= zh_pieces[i][pos][piece];
			SET0(pieces, pos);
		}

		ull whiteCastle = (board.castle[WHITE_SHORT] << 1) | board.castle[WHITE_LONG];
		ull blackCastle = (board.castle[BLACK_SHORT] << 1) | board.castle[BLACK_LONG];

		hashes[i] ^= zh_castle[i][whiteCastle][WHITE];
		hashes[i] ^= zh_castle[i][blackCastle][BLACK];
	}
	
	return { hashes[0], hashes[1], hashes[2] };
}
