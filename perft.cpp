#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <future>
#include <thread>

#include "bitboard.h"
#include "maps.h"
#include "move.h"
#include "movegen.h"
#include "perft.h"

std::atomic<ull> answer;

perft::PerftResult perft::test(bitboard::Position &board, int depth) {
	ull start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	perft::perftAtomicFirst(bitboard::board, depth);
	ull end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return { (ull) answer, (int) (end - start) == 0 ? 1 : (int) (end - start) };
}

void perft::perftAtomicFirst(const bitboard::Position &board, int depth) {
	answer = 0;

	std::vector<int> moves;
	movegen::move_gen(board, moves);

	if (depth == 1) {
		answer += moves.size();
		return;
	}

	std::vector<std::thread> threads;
	for (const int &move : moves) {
		bitboard::Position newBoard;
		memcpy(&newBoard, &board, sizeof(board));
		move::make_move(newBoard, move);

		threads.push_back(std::thread(perft::perftAtomic, newBoard, depth - 1, move));
	}

	for (int i = 0; i < threads.size(); i++)
		threads[i].join();
}

void perft::perftAtomic(const bitboard::Position &board, int depth, int prevMove) {
	std::vector<int> moves;
	movegen::move_gen(board, moves);

	if (depth == 1) {
		answer += moves.size();
		return;
	}

	ull positions = 0;
	for (const int &move : moves) {
		bitboard::Position newBoard;
		memcpy(&newBoard, &board, sizeof(board));
		move::make_move(newBoard, move);
		positions += perft::perft(newBoard, depth - 1);
	}

	std::cout << move::to_string(prevMove) << ": " << positions << "\n";
	answer += positions;
}

ull perft::perft(const bitboard::Position &board, int depth) {
	std::vector<int> moves;
	movegen::move_gen(board, moves);

	if (depth == 1)
		return moves.size();

	ull positions = 0;

	for (const int &move : moves) {
		bitboard::Position newBoard;
		memcpy(&newBoard, &board, sizeof(board));
		move::make_move(newBoard, move);
		positions += perft::perft(newBoard, depth - 1);
	}

	return positions;
}