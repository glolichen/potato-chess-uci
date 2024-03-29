#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <future>
#include <thread>
#include <sstream>

#include "bitboard.h"
#include "maps.h"
#include "move.h"
#include "movegen.h"
#include "perft.h"

std::atomic<ull> answer;

perft::PerftResult perft::test(const bitboard::Position &board, int depth) {
	ull start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	perft::perft_atomic_first(board, depth);
	ull end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return { (ull) answer, (int) (end - start) == 0 ? 1 : (int) (end - start) };
}

void perft::perft_atomic_first(const bitboard::Position &board, int depth) {
	answer = 0;

	std::vector<int> moves;
	movegen::move_gen(board, moves);

	if (depth == 1) {
		std::stringstream ss;
		for (int move : moves)
			ss << move::to_string(move) << ": 1\n";
		std::cout << ss.str();
		answer += moves.size();
		return;
	}

	std::vector<std::thread> threads;
	for (const int &move : moves) {
		bitboard::Position newBoard(board);
		move::make_move(newBoard, move);

		threads.push_back(std::thread(perft::perft_atomic, newBoard, depth - 1, move));
	}

	for (size_t i = 0; i < threads.size(); i++)
		threads[i].join();
}

void perft::perft_atomic(const bitboard::Position &board, int depth, int prevMove) {
	std::vector<int> moves;
	movegen::move_gen(board, moves);

	if (depth == 1) {
		std::stringstream ss;
		ss << move::to_string(prevMove) << ": " << moves.size() << "\n";
		std::cout << ss.str();
		answer += moves.size();
		return;
	}

	ull positions = 0;
	for (const int &move : moves) {
		bitboard::Position newBoard(board);
		move::make_move(newBoard, move);
		positions += perft::perft(newBoard, depth - 1);
	}

	std::stringstream ss;
	ss << move::to_string(prevMove) << ": " << positions << "\n";
	std::cout << ss.str();
	answer += positions;
}

ull perft::perft(const bitboard::Position &board, int depth) {
	std::vector<int> moves;
	movegen::move_gen(board, moves);

	if (depth == 1)
		return moves.size();

	ull positions = 0;

	for (const int &move : moves) {
		bitboard::Position newBoard(board);
		move::make_move(newBoard, move);
		positions += perft::perft(newBoard, depth - 1);
	}

	return positions;
}