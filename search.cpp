#include <chrono>
#include <cstring>
#include <iostream>
#include <limits.h>
#include <unordered_map>
#include <tuple>
#include <queue>
#include <vector>
#include <future>
#include <thread>

#include "bitboard.h"
#include "eval.h"
#include "hash.h"
#include "maps.h"
#include "movegen.h"
#include "search.h"

#include "ctpl_stl.h"

/*
add soon:
 * transposition table on quiscence
 * play test games
 * std map to unordered map
*/

#define DELTA_CUTOFF 800
#define QUISCENCE_DEPTH 5
#define TT_KEEP_MOVE_COUNT 5
#define NODES_PER_TIME_CHECK 8192
#define THREAD_COUNT 8
#define INT_MIN_PLUS_1 (INT_MIN + 1)

const int SEARCH_EXPIRED = INT_MIN_PLUS_1 + 500;

struct TTResult {
	int eval, move, depth, moveNum;
};

ctpl::thread_pool tp(THREAD_COUNT);

bool topMoveNull;
int bestMove;
int secondBestEval;

int moveNum = 0;

std::mutex mx;
std::unordered_map<ull, TTResult> transposition;

void table_insert(ull hash, TTResult result) {
	mx.lock();
	transposition.insert({ hash, result });
	mx.unlock();
}

void search::table_clear() {
	moveNum++;
	// https://stackoverflow.com/a/8234813
	for (auto it = transposition.begin(); it != transposition.end();) {
		if (it->second.moveNum >= moveNum - TT_KEEP_MOVE_COUNT)
			transposition.erase(it++);
		else
			it++;
	}
}

ull limit;
ull nodes;
bool is_time_up() {
	ull now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return now >= limit;
}

// algorithm from: https://www.chessprogramming.org/Quiescence_Search
int quiescence(const bitboard::Position &board, int alpha, int beta, int depth) {
	nodes++;
	if ((nodes & (NODES_PER_TIME_CHECK - 1)) == (NODES_PER_TIME_CHECK - 1) && is_time_up())
		return SEARCH_EXPIRED;

	int doNothingScore = eval::evaluate(board) * (board.turn ? -1 : 1);
	if (doNothingScore >= beta)
		return beta;

	if (alpha <= INT_MIN_PLUS_1 + 1 && alpha >= INT_MAX - 1 && doNothingScore < alpha - DELTA_CUTOFF)
		return alpha;

	alpha = std::max(doNothingScore, alpha);
	
	std::vector<int> moves;
	movegen::move_gen_with_ordering(board, moves);

	for (const int &move : moves) {
		if (board.mailbox[DEST(move)] == -1)
			continue;

		bitboard::Position newBoard;
		memcpy(&newBoard, &board, sizeof(board));
		move::make_move(newBoard, move);

		int score = quiescence(newBoard, -beta, -alpha, depth - 1);
		if (score == SEARCH_EXPIRED)
			return SEARCH_EXPIRED;
		score *= -1;

		if (score >= beta)
			return beta;
		alpha = std::max(score, alpha);
	}

	return alpha;
}

void search::pvs(int &result, const bitboard::Position &board, int depth, int alpha, int beta, int depth_from_start, bool use_threads) {
	nodes++;
	if ((nodes & (NODES_PER_TIME_CHECK - 1)) == (NODES_PER_TIME_CHECK - 1) && is_time_up()) {
		result = SEARCH_EXPIRED;
		return;
	}

	std::vector<int> moves;
	movegen::move_gen_with_ordering(board, moves);

	if (depth_from_start == 0 && !topMoveNull) {
		for (size_t i = 0; i < moves.size(); i++) {
			if (moves[i] == bestMove) {
				moves.erase(moves.begin() + i);
				break;
			}
		}
		moves.insert(moves.begin(), bestMove);
	}

	if (moves.size() == 0) {
		result = movegen::get_checks(board, board.turn) ? INT_MIN_PLUS_1 + depth_from_start + 1 : 0;
		return;
	}

	if (depth == 0) {		
		result = quiescence(board, INT_MIN_PLUS_1, INT_MAX, QUISCENCE_DEPTH);
		// result = eval::evaluate(board) * (board.turn ? -1 : 1);
		return;
	}

	if (board.fiftyMoveClock >= 50) {
		result = 0;
		return;
	}

	ull hash = hash::get_hash(board);
	if (depth_from_start && depth_from_start <= 4 && bitboard::prevPositions.count(hash)) {
		result = 0;
		return;
	}
	auto it = transposition.find(hash);
	if (it != transposition.end()) {
		bool contains = false;
		for (size_t i = 0; i < moves.size(); i++) {
			if (moves[i] == it->second.move) {
				moves.erase(moves.begin() + i);
				moves.insert(moves.begin(), it->second.move);
				contains = true;
				break;
			}
		}
		if (contains && it->second.depth > depth && depth_from_start) {
			result = it->second.eval * (board.turn ? -1 : 1);
			return;
		}
	}

	int *data;
	std::vector<std::future<void>> results;
	if (use_threads)
		data = new int[moves.size()];

	int topMove = moves[0];
	for (size_t i = 0; i < moves.size(); i++) {
		bitboard::Position newBoard;
		memcpy(&newBoard, &board, sizeof(board));
		if (board.mailbox[DEST(moves[i])] != -1 || board.mailbox[SOURCE(moves[i])] == PAWN || board.mailbox[SOURCE(moves[i])] == PAWN + 6)
			newBoard.fiftyMoveClock = 0;
		else
			newBoard.fiftyMoveClock++;
		move::make_move(newBoard, moves[i]);
		
		int curEval;
		if (i == 0) {
			search::pvs(curEval, newBoard, depth - 1, -beta, -alpha, depth_from_start + 1, use_threads);
			if (curEval == SEARCH_EXPIRED) {
				result = SEARCH_EXPIRED;
				goto end;
			}
			curEval *= -1;
		}
		else {
			if (use_threads) {
				results.push_back(tp.push([&data, i, newBoard, depth, alpha, beta, depth_from_start](int) {
					search::pvs(data[i], newBoard, depth - 1, -alpha - 1, -alpha, depth_from_start + 1, false);
					data[i] *= -1;
					if (data[i] > alpha) {
						search::pvs(data[i], newBoard, depth - 1, -beta, -data[i], depth_from_start + 1, false);
						data[i] *= -1;
					}
				}));
				continue;
			}
			else {
				search::pvs(curEval, newBoard, depth - 1, -alpha - 1, -alpha, depth_from_start + 1, false);
				if (curEval == SEARCH_EXPIRED) {
					result = SEARCH_EXPIRED;
					return;
				}
				curEval *= -1;
				if (curEval > alpha) {
					search::pvs(curEval, newBoard, depth - 1, -beta, -curEval, depth_from_start + 1, false);
					if (curEval == SEARCH_EXPIRED) {
						result = SEARCH_EXPIRED;
						return;
					}
					curEval *= -1;
				}
			}
		}

		if (curEval > alpha) {
			if (depth_from_start == 0)
				secondBestEval = alpha;
			alpha = curEval;
			topMove = moves[i];
		}
		if (alpha >= beta)
			break;
	}

	if (use_threads) {
		for (size_t i = 0; i < results.size(); i++)
			results[i].get();
		for (size_t i = 1; i < moves.size(); i++) {
			if (data[i] == -SEARCH_EXPIRED) {
				result = SEARCH_EXPIRED;
				goto end;
			}

			if (data[i] > alpha) {
				if (depth_from_start == 0)
					secondBestEval = alpha;
				alpha = data[i];
				topMove = moves[i];
			}
			if (alpha >= beta)
				break;
		}
	}

	it = transposition.find(hash);
	if (it == transposition.end())
		table_insert(hash, { alpha * (board.turn ? -1 : 1), topMove, depth, moveNum });
	else {
		if (depth > it->second.depth) {
			mx.lock();
			transposition[hash] = { alpha * (board.turn ? -1 : 1), topMove, depth, moveNum };
			mx.unlock();
		}
	}

	if (depth_from_start == 0)
		bestMove = topMove;

	result = alpha;
	
	end:
	if (use_threads)
		delete[] data;
}

void timer_thread(int) {
	std::string token;
	while (true) {
		std::cin >> token;
		if (token == "stop") {
			limit = 0;
			break;
		}
	}
}

search::SearchResult search::search_by_time(bitboard::Position &board, int time_MS, bool full_search) {
	// iterative deepening
	// search with depth of one ply first
	// then increase depth until time runs out
	// if time runs out in the middle of a search, terminate the search
	// and use the result from the previous search

	// it may seem like that the program is wasting a lot of time searching previously searched positions
	// but we can use alpha and beta values from before to speed up pruning
	// and we can search the best move first in the deeper search

	topMoveNull = true;
	secondBestEval = INT_MIN;

	limit = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	limit += time_MS;
	
	int depth = 3;
	bool extensionBonus = false;
	int eval = 0, prevEval = 0, prevBestMove = 0;

	while (true) {
		nodes = 0, prevEval = eval, prevBestMove = bestMove;
		search::pvs(eval, board, depth, INT_MIN_PLUS_1 + 2, INT_MAX - 2, 0, true);
		topMoveNull = false;

		if (eval == SEARCH_EXPIRED) {
			depth--;
			break;
		}

		std::cout << "info depth " << std::to_string(depth) << " nodes " << nodes << " currmove " << move::to_string(bestMove) << " score ";

		// checkmate has been found, do not need to search any more
		int isMate = search::eval_is_mate(eval);
		if (isMate != -1) {
			std::cout << "mate ";
			if (eval > 0) {
				std::cout << (int) ((INT_MAX - eval) / 2.0 + 0.5) << "\n";
				break;
			}
			else
				std::cout << "-" << (int) ((eval - INT_MIN_PLUS_1 - 1) / 2.0 + 0.5) << "\n";
		}
		else
			std::cout << "cp " << eval << "\n";

		if (!full_search && prevBestMove == bestMove && eval < 900) {
			// if everything has been pruned through move ordering
			// the move must be very good for that to happen
			// when this happens, secondBestEval is set to -2147483645, not INT_MIN or INT_MIN_PLUS_1???
			// don't know why, but it works
			if (depth >= 6 && (secondBestEval == -2147483645))
				break;
			// if (depth >= 6 && (eval - secondBestEval > 150 || std::abs(eval - prevEval) < 60))
			// 	break;
			// if (depth >= 5 && (eval - secondBestEval > 100 && std::abs(eval - prevEval) < 60))
			// 	break;
			// if (depth >= 5 && eval - secondBestEval > 250)
			// 	break;
		}
		if (depth >= 5 && bestMove != prevBestMove && std::abs(eval - prevEval) > 100 && !extensionBonus) {
			extensionBonus = true;
			limit += 1.5 * time_MS;
		}

		depth++;
	}

	return { bestMove, extensionBonus ? depth + 100 : depth, eval };
}

search::SearchResult search::search_by_depth(bitboard::Position &board, int depth) {
	limit = ULLONG_MAX;
	
	int eval = 0;
	search::pvs(eval, board, depth, INT_MIN_PLUS_1 + 2, INT_MAX - 2, 0, true);

	std::cout << "info depth " << std::to_string(depth) << " nodes " << nodes << " currmove " << move::to_string(bestMove) << " score ";

	int isMate = search::eval_is_mate(eval);
	if (isMate != -1) {
		std::cout << "mate ";
		if (eval > 0)
			std::cout << (int) ((INT_MAX - eval) / 2.0 + 0.5) << "\n";
		else
			std::cout << "-" << (int) ((eval - INT_MIN_PLUS_1 - 1) / 2.0 + 0.5) << "\n";
	}
	else
		std::cout << "cp " << eval << "\n";

	return { bestMove, depth, eval };
}

search::SearchResult search::search_unlimited(bitboard::Position &board) {
	topMoveNull = true;

	limit = ULLONG_MAX;
	tp.push(timer_thread);
	
	int depth = 3, eval = 0;
	while (true) {
		search::pvs(eval, board, depth, INT_MIN_PLUS_1 + 2, INT_MAX - 2, 0, true);
		topMoveNull = false;

		if (eval == SEARCH_EXPIRED) {
			depth--;
			break;
		}

		std::cout << "info depth " << std::to_string(depth) << " nodes " << nodes << " currmove " << move::to_string(bestMove) << " score ";

		int isMate = search::eval_is_mate(eval);
		if (isMate != -1) {
			std::cout << "mate ";
			if (eval > 0)
				std::cout << (int) ((INT_MAX - eval) / 2.0 + 0.5) << "\n";
			else
				std::cout << "-" << (int) ((eval - INT_MIN_PLUS_1 - 1) / 2.0 + 0.5) << "\n";
		}
		else
			std::cout << "cp " << eval << "\n";

		depth++;
	}

	return { bestMove, depth, eval };
}

int search::eval_is_mate(int eval) {
	if (eval > INT_MAX - 250)
		return WHITE;
	if (eval < INT_MIN_PLUS_1 + 250)
		return BLACK;
	return -1;
}