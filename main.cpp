#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <format>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <istream>

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

#define LOG_DIR "/home/jayden/Desktop/Programs/potato-chess/logs/"

const std::string START_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// https://stackoverflow.com/a/674120
class OutputStream : public std::ostream {
private:
	bool is_logging;
public:
	OutputStream(std::string file) {
		is_logging = true;
		std::freopen(file.c_str(), "w", stderr);
	}
	OutputStream() {
		is_logging = false;
	}

	// log output
	template <class T>
	OutputStream &operator<<(const T &v) {
		std::cout << v;
		if (is_logging)
			std::cerr << time(0) << " O: " << v;
		return *this;
	}

	// log input
	void log_input(const std::string text) {
		if (is_logging)
			std::cerr << time(0) << " I: " << text << "\n";
	}
};

OutputStream *outputter;

int main() {
	#ifdef LOG_DIR
		std::stringstream log_file_dir;
		log_file_dir << LOG_DIR << "log" << time(0) << ".txt";
		outputter = new OutputStream(log_file_dir.str());
	#else
		outputter = new OutputStream();
	#endif

	srand(time(0));

	maps::init();
	eval::init();
	hash::init();

	book::book_open("/home/jayden/Desktop/Programs/potato-chess/potato-chess-uci/books/Book.bin");
	
	bitboard::Position oldBoard(START_POS);
	bitboard::Position board(START_POS);

	std::unordered_map<std::string, int> options;
	int totalHalfMoves = 0;

	search::SearchResult lastResult = { -1, -1, -1, -1 };

	while (true) {
		skip:
		
		std::string line;
		getline(std::cin, line);

		outputter->log_input(line);

		std::stringstream ss(line);
		
		std::string token;
		ss >> token;

		if (token == "isready")
			*outputter << "readyok\n";
		else if (token == "uci")
			*outputter << "id name Potato Chess\nid author Jayden Li\noption name Move Overhead type spin default 10 min 0 max 5000\nuciok\n";
		else if (token == "position") {
			totalHalfMoves = 0;
			ss >> token;
			if (token == "startpos")
				board.decode(START_POS);
			else if (token == "fen") {
				std::string fen = "";
				for (int i = 0; i < 6; i++) {
					ss >> token;
					fen += token + " ";
				}
				board.decode(fen);
			}


			bitboard::prevPositions.emplace(hash::get_hash(board));
			if (!ss.eof()) {
				ss >> token;
				if (token == "moves") {
					while (!ss.eof()) {
						ss >> token;
						int move = move::uci_to_move(board, token);
						move::make_move(board, move);
						bitboard::prevPositions.emplace(hash::get_hash(board));
						totalHalfMoves++;
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
					perft::PerftResult result = perft::test(board, depth);
					*outputter << "total nodes: " << result.totalNodes << "\n";
					*outputter << "time: " << result.time << "ms\n";
					*outputter << "nodes per second: " << (result.totalNodes / result.time * 1000) << "\n";
					goto skip;
				}
				else if (token == "depth")
					ss >> depth;
				else if (token == "ponder")
					ponder = true;
			}

			search::SearchResult result;

			if (ponder) {
				int remainingTime = !board.turn ? btime : wtime;
				int inc = !board.turn ? binc : winc;
				if (inc == -1)
					inc = 0;

				oldBoard = board;
				move::make_move(board, lastResult.move);
				move::make_move(board, lastResult.ponder);

				int time = timeman::calc_base_time(remainingTime, totalHalfMoves / 2) - options["Move Overhead"];
				time = std::max(100, time);
				result = search::ponder(board, time);

				// *outputter << result.move << "\n";
				if (result.move == -1) {
					board = oldBoard;
					*outputter << "bestmove " << move::to_string(lastResult.move);
					*outputter << " ponder " << move::to_string(lastResult.ponder) << "\n";
					goto skip;
				}
			}
			else {
				int bookMove = book::book_move(board);
				if (bookMove != -1) {
					*outputter << "bestmove " << move::to_string(bookMove) << "\n";
					continue;
				}
				
				if (movetime != -1)
					result = search::search_by_time(board, movetime, true);
				else if (depth != -1)
					result = search::search_by_depth(board, depth);
				else if (wtime != -1 && btime != -1) {
					int remainingTime = board.turn ? btime : wtime;
					int inc = board.turn ? binc : winc;
					if (inc == -1)
						inc = 0;

					int time = timeman::calc_base_time(remainingTime, totalHalfMoves / 2) - options["Move Overhead"];
					time = std::max(100, time);
					result = search::search_by_time(board, time + (inc * 0.5), false);

					search::table_clear_move();
				}
				else
					result = search::search_unlimited(board);
			}
			int bestMove = result.move, ponderMove = result.ponder;

			lastResult = result;

			*outputter << "bestmove " << move::to_string(bestMove);
			if (ponderMove != -1 && ponderMove != 0)
				*outputter << " ponder " << move::to_string(ponderMove) << "\n";
			else
				*outputter << "\n";
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
			*outputter << book::gen_polyglot_key(board) << "\n";
		else if (token == "bookmove") {
			int move = book::book_move(board);
			if (move == -1)
				*outputter << "no book move found for this position\n";
			else
				*outputter << move::to_string(move) << "\n";
		}
		else if (token == "print")
			board.print();
		else if (token == "quit")
			break;
		else if (token == "ucinewgame")
			search::table_clear();
	}

	return 0;
}