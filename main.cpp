#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <format>
#include <fstream>

#include "bitboard.h"
#include "eval.h"
#include "maps.h"
#include "hash.h"
#include "move.h"
#include "movegen.h"
#include "perft.h"
#include "search.h"
#include "book.h"

int main() {
	srand(time(0));

	maps::init();
	hash::init();

	book::book_open("/home/jayden/Desktop/Programs/potato-chess/potato-chess-uci/books/Book.bin");

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
			std::cout << "id name Potato Chess\nid author Jayden Li\nuciok\n";
		else if (token == "position") {
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

						// std::cout << move::to_string(move) << "\n";
						// bitboard::print_board(bitboard::board);
					}
				}
			}
		}
		else if (token == "go") {			
			int wtime = -1, btime = -1, movetime = -1, depth = -1;
			while (ss >> token) {
				if (token == "wtime")
					ss >> wtime;
				else if (token == "btime")
					ss >> btime;
				else if (token == "movetime")
					ss >> movetime;
				else if (token == "perft") {
					int depth;
					ss >> depth;
					perft::PerftResult result = perft::test(bitboard::board, depth);
					std::cout << "\ntotal nodes: " << result.totalNodes << "\n";
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
				else
					res = search::search(bitboard::board, 3000);
				move = res.move;
			}

			std::cout << "bestmove ";
			std::cout << move::to_string(move) << "\n";
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
