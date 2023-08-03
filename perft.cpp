#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "bitboard.h"
#include "maps.h"
#include "move.h"
#include "movegen.h"
#include "perft.h"

perft::PerftResult perft::test(bitboard::Position &board, int depth) {
	ull start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	ull result = perft::perft(bitboard::board, depth, true);
	ull end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return { result, (int) (end - start) };
}

ull perft::perft(const bitboard::Position &board, int depth, bool first) {
	std::vector<int> moves;
	movegen::move_gen(board, moves);

	if (depth == 1)
		return moves.size();

	ull old = 0;
	ull positions = 0;

	for (const int &move : moves) {
		if (first) {
			std::cout << move::to_string(move) << ": ";
		}

		bitboard::Position new_board;
		memcpy(&new_board, &board, sizeof(board));
		move::make_move(new_board, move);
		positions += perft::perft(new_board, depth - 1, false);

		if (first) {
			std::cout << (positions - old) << "\n";
			old = positions;
		}
	}

	return positions;
}
