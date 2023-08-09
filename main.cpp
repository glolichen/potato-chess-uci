#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <format>
#include <fstream>
#include <unordered_map>

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
	hash::init();

	// bitboard::decode("8/8/8/8/nk1p1RB1/8/4P3/K7 w - - 0 1");
	// move::make_move(bitboard::board, 46784);
	// // bitboard::decode("8/2p5/8/1P1p3r/KR3p1k/8/4P1P1/8 w - - 0 2");
	// // move::make_move(bitboard::board, 38464);
	// bitboard::print_board(bitboard::board);

	// std::vector<int> moves;
	// movegen::move_gen(bitboard::board, moves);
	// for (int move : moves)
	// 	std::cout << move::to_string(move) << ", ";
	// std::cout << moves.size();

	// return 0;

	book::book_open("/home/jayden/Desktop/Programs/potato-chess/potato-chess-uci/books/Book.bin");

	std::unordered_map<std::string, int> options;
	int totalHalfMoves = 0;

	while (true) {
		skip:
		
		std::string line;
		getline(std::cin, line);

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


			if (!ss.eof()) {
				ss >> token;
				if (token == "moves") {
					while (!ss.eof()) {
						ss >> token;
						int move = move::uci_to_move(bitboard::board, token);
						move::make_move(bitboard::board, move);
						totalHalfMoves++;

						// std::cout << move::to_string(move) << "\n";
						// bitboard::print_board(bitboard::board);
					}
				}
			}
		}
		else if (token == "go") {			
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
			}

			int bookMove = book::book_move(bitboard::board);
			if (bookMove != -1) {
				std::cout << "bestmove " << move::to_string(bookMove) << "\n";
				continue;
			}

			std::vector<int> moves;
			movegen::move_gen(bitboard::board, moves);

			search::SearchResult res;
			int move;
			
			if (moves.size() == 1)
				move = moves[0];
			else {
				if (movetime != -1)
					res = search::search(bitboard::board, movetime);
				else if (depth != -1)
					res = search::search(bitboard::board, -depth);
				else {
					int remainingTime = bitboard::board.turn ? btime : wtime;
					int inc = bitboard::board.turn ? binc : winc;
					if (inc == -1)
						inc = 0;

					int time = timeman::calc_base_time(remainingTime, totalHalfMoves / 2) - options["Move Overhead"];
					time = std::max(100, time);
					res = search::search(bitboard::board, time + (inc * 0.5));
				}
				move = res.move;
			}

			std::cout << "bestmove ";
			std::cout << move::to_string(move) << "\n";
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
	}

	return 0;
}
