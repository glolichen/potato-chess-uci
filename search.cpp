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
#define TT_SIZE_MB 512
#define NODES_PER_TIME_CHECK 8192
#define THREAD_COUNT 8

const int SEARCH_EXPIRED = INT_MIN + 500;

struct TTResult {
	int eval, move, depth;
};

ctpl::thread_pool tp(THREAD_COUNT);

bool topMoveNull;
int bestMove;
int secondBestEval;

// std::tuple as key in unordered map: https://stackoverflow.com/a/20835070
std::mutex mx;
std::unordered_map<ull, TTResult> transposition;
std::queue<ull> hashKeys;

const ull maxSize = TT_SIZE_MB * 1024 * 1024;
/*
let a = size of zobrist tuple
let b = size of ttresult
let k = max size
let x = max entries

S_transposition = x(a + b) = ax + bx
S_hashKeys = ax

S_transposition + S_hashKeys = k
ax + bx + ax = k
2ax + bx = k
x(2a + b) = k
x = k/(2a + b)
*/
const int maxElements = maxSize / (2 * sizeof(ull) + sizeof(TTResult));

void table_insert(ull hashes, TTResult result) {
	mx.lock();
	if (transposition.size() >= maxElements) {
		ull out = hashKeys.back();
		hashKeys.pop();
		transposition.erase(out);
	}
	transposition.insert({ hashes, result });
	hashKeys.push(hashes);
	mx.unlock();
}

ull limit;

ull nodes;
bool is_time_up() {
	ull now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	if (now >= limit)
		return true;
	return false;
}

// algorithm from: https://www.chessprogramming.org/Quiescence_Search
int quiescence(const bitboard::Position &board, int alpha, int beta, int depth) {
	nodes++;
	if ((nodes & (NODES_PER_TIME_CHECK - 1)) == (NODES_PER_TIME_CHECK - 1) && is_time_up())
		return SEARCH_EXPIRED;

	int doNothingScore = eval::evaluate(board) * (board.turn ? -1 : 1);
	if (doNothingScore >= beta)
		return beta;

	if (alpha <= INT_MIN + 1 && alpha >= INT_MAX - 1 && doNothingScore < alpha - DELTA_CUTOFF)
		return alpha;

	alpha = std::max(doNothingScore, alpha);
	
	std::vector<int> moves;
	movegen::move_gen_with_ordering(board, moves);

	for (const int &move : moves) {
		if (board.mailbox[DEST(move)] == -1)
			continue;

		bitboard::Position new_board;
		memcpy(&new_board, &board, sizeof(board));
		move::make_move(new_board, move);

		int score = quiescence(new_board, -beta, -alpha, depth - 1);
		if (score == SEARCH_EXPIRED)
			return SEARCH_EXPIRED;
		score *= -1;

		if (score >= beta)
			return beta;
		alpha = std::max(score, alpha);
	}

	return alpha;
}

void search::pvs(int &result, const bitboard::Position &board, int depth, int alpha, int beta, int depthFromStart, bool useThreads) {
	nodes++;
	if ((nodes & (NODES_PER_TIME_CHECK - 1)) == (NODES_PER_TIME_CHECK - 1) && is_time_up()) {
		result = SEARCH_EXPIRED;
		return;
	}

	std::vector<int> moves;
	movegen::move_gen_with_ordering(board, moves);

	if (depthFromStart == 0 && !topMoveNull) {
		for (size_t i = 0; i < moves.size(); i++) {
			if (moves[i] == bestMove) {
				moves.erase(moves.begin() + i);
				break;
			}
		}
		moves.insert(moves.begin(), bestMove);
	}

	if (moves.size() == 0) {
		result = movegen::get_checks(board, board.turn) ? INT_MIN + depthFromStart + 1 : 0;
		return;
	}

	if (depth == 0) {		
		result = quiescence(board, INT_MIN, INT_MAX, QUISCENCE_DEPTH);
		// result = eval::evaluate(board) * (board.turn ? -1 : 1);
		return;
	}

	if (board.fiftyMoveClock >= 50) {
		result = 0;
		return;
	}

	ull hash = hash::get_hash(board);
	if (depthFromStart && depthFromStart <= 4 && bitboard::prevPositions.count(hash)) {
		result = 0;
		return;
	}
	if (transposition.count(hash)) {
		TTResult prev = transposition.at(hash);
		if (prev.depth > depth && depthFromStart) {
			result = prev.eval * (board.turn ? -1 : 1);
			return;
		}
		for (size_t i = 0; i < moves.size(); i++) {
			if (moves[i] == prev.move) {
				moves.erase(moves.begin() + i);
				moves.insert(moves.begin(), prev.move);
				break;
			}
		}
	}

	int *data;
	std::vector<std::future<void>> results;
	if (useThreads)
		data = new int[moves.size()];

	int topMove = moves[0], score = INT_MIN;
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
			search::pvs(curEval, newBoard, depth - 1, -beta, -alpha, depthFromStart + 1, useThreads);
			if (curEval == SEARCH_EXPIRED) {
				result = SEARCH_EXPIRED;
				goto end;
			}
			curEval *= -1;
		}
		else {
			if (useThreads) {
				results.push_back(tp.push([&data, i, newBoard, depth, alpha, beta, depthFromStart](int) {
					search::pvs(data[i], newBoard, depth - 1, -alpha - 1, -alpha, depthFromStart + 1, false);
					data[i] *= -1;
					if (data[i] > alpha) {
						search::pvs(data[i], newBoard, depth - 1, -beta, -data[i], depthFromStart + 1, false);
						data[i] *= -1;
					}
				}));
				continue;
			}
			else {
				search::pvs(curEval, newBoard, depth - 1, -alpha - 1, -alpha, depthFromStart + 1, false);
				if (curEval == SEARCH_EXPIRED) {
					result = SEARCH_EXPIRED;
					return;
				}
				curEval *= -1;
				if (curEval > alpha) {
					search::pvs(curEval, newBoard, depth - 1, -beta, -curEval, depthFromStart + 1, false);
					if (curEval == SEARCH_EXPIRED) {
						result = SEARCH_EXPIRED;
						return;
					}
					curEval *= -1;
				}
			}
		}

		if (curEval > alpha) {
			secondBestEval = alpha;
			alpha = curEval;
			topMove = moves[i];
		}
		if (alpha >= beta)
			break;
	}

	if (useThreads) {
		for (size_t i = 0; i < results.size(); i++)
			results[i].get();
		for (size_t i = 1; i < moves.size(); i++) {
			if (data[i] == -SEARCH_EXPIRED) {
				result = SEARCH_EXPIRED;
				goto end;
			}

			if (data[i] > alpha) {
				secondBestEval = alpha;
				alpha = data[i];
				topMove = moves[i];
			}
			if (alpha >= beta)
				break;
		}
	}

	if (transposition.count(hash) == 0)
		table_insert(hash, { score * (board.turn ? -1 : 1), topMove, depth });
	else {
		TTResult prev = transposition.at(hash);
		if (depth > prev.depth) {
			mx.lock();
			transposition[hash] = { score * (board.turn ? -1 : 1), topMove, depth };
			mx.unlock();
		}
	}

	if (depthFromStart == 0)
		bestMove = topMove;

	result = alpha;
	
	end:
	if (useThreads)
		delete[] data;
}

search::SearchResult search::search(bitboard::Position &board, int timeMS) {
	// iterative deepening
	// search with depth of one ply first
	// then increase depth until time runs out
	// if time runs out in the middle of a search, terminate the search
	// and use the result from the previous search

	// it may seem like that the program is wasting a lot of time searching previously searched positions
	// but we can use alpha and beta values from before to speed up pruning
	// and we can search the best move first in the deeper search

	int time, searchDepth = -1;
	if (timeMS < 0) {
		time = 10000000;
		searchDepth = -timeMS;
	}
	else
		time = timeMS;

	topMoveNull = true;

	limit = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	limit += time;
	
	std::pair<int, int> best;

	int depth = 1;
	if (searchDepth != -1)
		depth = searchDepth;
	bool is_mate = false;

	bool extensionBonus = false;
	int eval = 0, prevEval = 0, prevBestMove = 0;
	while (true) {
		nodes = 0, prevEval = eval, prevBestMove = bestMove;
		search::pvs(eval, board, depth, INT_MIN + 2, INT_MAX - 2, 0, true);
		topMoveNull = false;

		if (eval == SEARCH_EXPIRED) {
			depth--;
			break;
		}

		std::cout << "info depth " << std::to_string(depth) << " nodes " << nodes << " currmove " << move::to_string(bestMove) << " score ";

		best.first = bestMove;
		best.second = eval;

		// checkmate has been found, do not need to search any more
		int isMate = eval_is_mate(eval);
		if (isMate != -1) {
			std::cout << "mate ";
			if (eval > 0)
				std::cout << (int) ((INT_MAX - eval) / 2.0 + 0.5);
			else
				std::cout << "-" << (int) ((eval - INT_MIN - 1) / 2.0 + 0.5);
			std::cout << "\n";

			if (isMate == board.turn) {
				is_mate = true;
				break;
			}
		}
		else
			std::cout << "cp " << eval << "\n";

		if (searchDepth != -1)
			break;

		if (prevBestMove == bestMove) {
			if (depth >= 6 && ((eval > 500 && prevEval > 500) || std::abs(eval - prevEval) < 40))
				break;
			if (depth >= 5 && (bestMove - secondBestEval > 300 || std::abs(eval - prevEval) < 20))
				break;
		}
		if (depth >= 5 && bestMove != prevBestMove && std::abs(eval - prevEval) > 200 && !extensionBonus)
			limit += 2 * time;

		depth++;
	}

	// transposition.clear();

	return { best.first, depth, best.second, is_mate };
}

int search::eval_is_mate(int eval) {
	if (eval > INT_MAX - 250)
		return WHITE;
	if (eval < INT_MIN + 250)
		return BLACK;
	return -1;
}