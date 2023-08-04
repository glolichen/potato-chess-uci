#include <chrono>
#include <cstring>
#include <iostream>
#include <limits.h>
#include <unordered_map>
#include <tuple>
#include <queue>
#include <vector>

#include "bitboard.h"
#include "eval.h"
#include "hash.h"
#include "maps.h"
#include "movegen.h"
#include "search.h"

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

const int SEARCH_EXPIRED = INT_MIN + 500;

struct TTResult {
	int eval, move, depth;
};

bool topMoveNull;
int bestMove, secondBestMove;

// std::tuple as key in unordered map: https://stackoverflow.com/a/20835070
std::unordered_map<hashdefs::ZobristTuple, TTResult, hashdefs::ZobristTupleHash> transposition;
std::queue<hashdefs::ZobristTuple> hashKeys;

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
const int maxElements = maxSize / (2 * sizeof(hashdefs::ZobristTuple) + sizeof(TTResult));

void table_insert(hashdefs::ZobristTuple hashes, TTResult result) {
	if (transposition.size() >= maxElements) {
		hashdefs::ZobristTuple out = hashKeys.back();
		hashKeys.pop();
		transposition.erase(out);
	}
	transposition.insert({ hashes, result });
	hashKeys.push(hashes);
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
	if (nodes & (NODES_PER_TIME_CHECK - 1) == (NODES_PER_TIME_CHECK - 1) && is_time_up())
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

int search::minimax(const bitboard::Position &board, int depth, int alpha, int beta, int depth_from_start) {
	nodes++;
	if (nodes & (NODES_PER_TIME_CHECK - 1) == (NODES_PER_TIME_CHECK - 1) && is_time_up())
		return SEARCH_EXPIRED;

	std::vector<int> moves;
	movegen::move_gen_with_ordering(board, moves);

	if (depth_from_start == 0 && !topMoveNull) {
		for (int i = 0; i < moves.size(); i++) {
			if (moves[i] == bestMove) {
				moves.erase(moves.begin() + i);
				break;
			}
		}
		moves.insert(moves.begin(), bestMove);
	}

	if (moves.size() == 0) {
		if (movegen::get_checks(board, board.turn)) {
			int multiply = board.turn ? 1 : -1;
			return INT_MAX * multiply - multiply * depth_from_start; // checkmate
		}
		return 0;
	}

	if (depth == 0) {		
		int qsResult = quiescence(board, INT_MIN, INT_MAX, QUISCENCE_DEPTH);
		if (qsResult == SEARCH_EXPIRED)
			return SEARCH_EXPIRED;
		return qsResult * (board.turn ? -1 : 1);

		// return eval::evaluate(board);
	}

	if (board.fiftyMoveClock >= 50)
		return 0;

	hashdefs::ZobristTuple hashes = hash::hash(board);
	if (transposition.count(hashes)) {
		TTResult prev = transposition.at(hashes);
		if (prev.depth > depth)
			return prev.eval;
		for (int i = 0; i < moves.size(); i++) {
			if (moves[i] == prev.move) {
				moves.erase(moves.begin() + i);
				moves.insert(moves.begin(), prev.move);
				break;
			}
		}
	}

	int topMove, evaluation = board.turn ? INT_MAX : INT_MIN;
	for (const int &move : moves) {
		bitboard::Position new_board;
		memcpy(&new_board, &board, sizeof(board));
		if (board.mailbox[DEST(move)] != -1 || board.mailbox[SOURCE(move)] == PAWN || board.mailbox[SOURCE(move)] == PAWN + 6)
			new_board.fiftyMoveClock = 0;
		else
			new_board.fiftyMoveClock++;
		move::make_move(new_board, move);

		int cur_eval = search::minimax(new_board, depth - 1, alpha, beta, depth_from_start + 1);
		if (cur_eval == SEARCH_EXPIRED)
			return SEARCH_EXPIRED;

		if (!board.turn) { // white
			if (cur_eval > evaluation) {
				evaluation = cur_eval;
				topMove = move;
			}

			if (evaluation >= beta)
				break;
			alpha = std::max(evaluation, alpha);
		}
		else { // black
			if (cur_eval < evaluation) {
				evaluation = cur_eval;
				topMove = move;
			}

			if (evaluation <= alpha)
				break;
			beta = std::min(evaluation, beta);
		}
	}

	if (transposition.count(hashes) == 0)
		table_insert(hashes, { evaluation, topMove, depth });
	else {
		TTResult prev = transposition.at(hashes);
		if (depth > prev.depth)
			transposition[hashes] = { evaluation, topMove, depth };
	}

	if (depth_from_start == 0)
		bestMove = topMove;

	return evaluation;
}

search::SearchResult search::search(bitboard::Position &board, int time_MS) {
	// iterative deepening
	// search with depth of one ply first
	// then increase depth until time runs out
	// if time runs out in the middle of a search, terminate the search
	// and use the result from the previous search

	// it may seem like that the program is wasting a lot of time searching previously searched positions
	// but we can use alpha and beta values from before to speed up pruning
	// and we can search the best move first in the deeper search

	int time, searchDepth = -1;
	if (time_MS < 0) {
		time = 10000000;
		searchDepth = -time_MS;
	}
	else
		time = time_MS;

	topMoveNull = true;

	eval::init();
	hash::init();
	maps::init();

	limit = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	limit += time;
	
	std::pair<int, int> best;

	int depth = 1;
	if (searchDepth != -1)
		depth = searchDepth;
	bool is_mate = false;

	while (true) {
		nodes = 0;
		int eval = search::minimax(board, depth, INT_MIN, INT_MAX, 0);
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
			if (eval > 0) {
				if (board.turn == BLACK)
					std::cout << "-";
				std::cout << (int) ((INT_MAX - eval) / 2.0 + 0.5);
			}
			else {
				if (board.turn == WHITE)
					std::cout << "-";
				std::cout << (int) ((eval - INT_MIN) / 2.0 + 0.5);
			}
			std::cout << "\n";

			if (isMate == board.turn) {
				is_mate = true;
				break;
			}
		}
		else {
			std::cout << "cp ";
			if (board.turn == BLACK)
				std::cout << -eval;
			else
				std::cout << eval;
			
			std::cout << "\n";
		}

		if (searchDepth != -1)
			break;

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