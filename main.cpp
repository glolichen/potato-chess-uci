#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <format>
#include <fstream>
#include <unordered_map>
#include <vector>

#include <cstdlib>

#include "bitboard.h"
#include "eval.h"
#include "maps.h"
#include "hash.h"
#include "move.h"
#include "movegen.h"
#include "perft.h"
#include "search.h"
#include "book.h"
#include "timeman.h"

int main() {
	srand(time(0));

	maps::init();
	eval::init();
	hash::init();

	book::book_open("/home/jayden/Desktop/Programs/potato-chess/potato-chess-uci/books/Book.bin");
	
	std::unordered_map<std::string, int> options;
	int totalHalfMoves = 0;

	search::SearchResult lastResult = { -1, -1, -1, -1 };

	while (true) {
		skip:
		
		std::string line;
		getline(std::cin, line);

		// std::cout << line << "\n";
		// if (line == "")
		// 	continue;

		std::stringstream ss(line);
		
		std::string token;
		ss >> token;

		if (token == "isready")
			std::cout << "readyok\n";
		else if (token == "uci")
			std::cout << "id name Potato Chess\nid author Jayden Li\noption name Move Overhead type spin default 10 min 0 max 5000\nuciok\n";
		else if (token == "position") {
			totalHalfMoves = 0;
			ss >> token;
			if (token == "startpos")
				bitboard::decode("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			else if (token == "fen") {
				std::string fen = "";
				for (int i = 0; i < 6; i++) {
					ss >> token;
					fen += token + " ";
				}
				bitboard::decode(fen);
			}


			bitboard::prevPositions.emplace(hash::get_hash(bitboard::board));
			if (!ss.eof()) {
				ss >> token;
				if (token == "moves") {
					while (!ss.eof()) {
						ss >> token;
						int move = move::uci_to_move(bitboard::board, token);
						move::make_move(bitboard::board, move);
						bitboard::prevPositions.emplace(hash::get_hash(bitboard::board));
						totalHalfMoves++;

						// std::cout << move::to_string(move) << "\n";
						// bitboard::print_board(bitboard::board);
					}
				}
			}
		}
		else if (token == "go") {
			bool ponder = false;
			int wtime = -1, btime = -1, winc = -1, binc = -1, movetime = -1, depth = -1;
			while (ss >> token) {
				if (token == "wtime")
					ss >> wtime;
				else if (token == "btime")
					ss >> btime;
				else if (token == "winc")
					ss >> winc;
				else if (token == "binc")
					ss >> binc;
				else if (token == "movetime")
					ss >> movetime;
				else if (token == "perft") {
					int depth;
					ss >> depth;
					perft::PerftResult result = perft::test(bitboard::board, depth);
					std::cout << "total nodes: " << result.totalNodes << "\n";
					std::cout << "time: " << result.time << "ms\n";
					std::cout << "nodes per second: " << (result.totalNodes / result.time * 1000) << "\n";
					goto skip;
				}
				else if (token == "depth")
					ss >> depth;
				else if (token == "ponder")
					ponder = true;
			}

			search::SearchResult result;

			if (ponder) {
				int remainingTime = !bitboard::board.turn ? btime : wtime;
				int inc = !bitboard::board.turn ? binc : winc;
				if (inc == -1)
					inc = 0;

				int time = timeman::calc_base_time(remainingTime, totalHalfMoves / 2) - options["Move Overhead"];
				time = std::max(100, time);
				result = search::ponder(bitboard::board, time);

				// std::cout << result.move << "\n";
				if (result.move == -1) {
					std::cout << "bestmove " << move::to_string(lastResult.move);
					std::cout << " ponder " << move::to_string(lastResult.ponder) << "\n";
					goto skip;
				}
			}
			else {
				int bookMove = book::book_move(bitboard::board);
				if (bookMove != -1) {
					std::cout << "bestmove " << move::to_string(bookMove) << "\n";
					continue;
				}
				
				if (movetime != -1)
					result = search::search_by_time(bitboard::board, movetime, true);
				else if (depth != -1)
					result = search::search_by_depth(bitboard::board, depth);
				else if (wtime != -1 && btime != -1) {
					int remainingTime = bitboard::board.turn ? btime : wtime;
					int inc = bitboard::board.turn ? binc : winc;
					if (inc == -1)
						inc = 0;

					int time = timeman::calc_base_time(remainingTime, totalHalfMoves / 2) - options["Move Overhead"];
					time = std::max(100, time);
					result = search::search_by_time(bitboard::board, time + (inc * 0.5), false);

					search::table_clear_move();
				}
				else
					result = search::search_unlimited(bitboard::board);
			}
			int bestMove = result.move, ponderMove = result.ponder;

			lastResult = result;

			std::cout << "bestmove " << move::to_string(bestMove);
			if (ponderMove != 0) {
				std::cout << " ponder " << move::to_string(ponderMove) << "\n";
				move::make_move(bitboard::board, bestMove);
				move::make_move(bitboard::board, ponderMove);
			}
		}
		else if (token == "setoption") {
			std::string optionName;
			int value;
			while (ss >> token) {
				if (token == "name") {
					bool first = true;
					while (ss >> token) {
						if (token == "value")
							break;
						if (!first)
							optionName += " ";
						first = false;
						optionName += token;
					}
				}
				ss >> value;
				options[optionName] = value;
			} 
		}
		// polyglot (opening book system) key
		else if (token == "pgkey")
			std::cout << book::gen_polyglot_key(bitboard::board) << "\n";
		else if (token == "bookmove") {
			int move = book::book_move(bitboard::board);
			if (move == -1)
				std::cout << "no book move found for this position\n";
			else
				std::cout << move::to_string(move) << "\n";
		}
		else if (token == "print")
			bitboard::print_board(bitboard::board);
		else if (token == "quit")
			break;
		else if (token == "ucinewgame")
			search::table_clear();
	}

	return 0;
}