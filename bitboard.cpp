#include <iostream>
#include <string>
#include <vector>

#include "bitboard.h"

std::string bitboard::squares[64] = {
	"h1", "g1", "f1", "e1", "d1", "c1", "b1", "a1",
	"h2", "g2", "f2", "e2", "d2", "c2", "b2", "a2",
	"h3", "g3", "f3", "e3", "d3", "c3", "b3", "a3",
	"h4", "g4", "f4", "e4", "d4", "c4", "b4", "a4",
	"h5", "g5", "f5", "e5", "d5", "c5", "b5", "a5",
	"h6", "g6", "f6", "e6", "d6", "c6", "b6", "a6",
	"h7", "g7", "f7", "e7", "d7", "c7", "b7", "a7",
	"h8", "g8", "f8", "e8", "d8", "c8", "b8", "a8",
};
std::string bitboard::pieces = "PNBRQKpnbrqk";
bitboard::Position bitboard::board;
std::unordered_set<ull> bitboard::prevPositions;

std::vector<std::string> bitboard::split(std::string str, char split_on) {
	std::vector<std::string> result;

	int left = 0;
	for (size_t i = 0; i < str.length(); i++) {
		if (str[i] == split_on) {
			result.push_back(str.substr(left, i - left));
			left = i + 1;
		}
	}

	result.push_back(str.substr(left, str.length() - left));
	return result;
}

void bitboard::decode(std::string fen) {
	for (int i = 0; i < 64; i++)
		board.mailbox[i] = -1;
	board.allPieces = 0ull;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 7; j++)
			board.pieces[i][j] = 0ull;
	}
	for (int i = 0; i < 4; i++)
		board.castle[i] = false;
	board.turn = 0;
	board.enPassant = -1;
	board.fiftyMoveClock = 0;
	board.fullMove = 0;

	std::vector<std::string> splitted = bitboard::split(fen, ' ');

	if (!splitted[1].compare("w"))
		board.turn = WHITE;
	else
		board.turn = BLACK;

	try {
		board.fiftyMoveClock = stoi(splitted[4]);
		board.fullMove = stoi(splitted[5]);
	}
	catch (...) { }


	std::string castle = splitted[2];
	if (castle.find('K') != std::string::npos)
		board.castle[WHITE_SHORT] = true;
	if (castle.find('Q') != std::string::npos)
		board.castle[WHITE_LONG] = true;
	if (castle.find('k') != std::string::npos)
		board.castle[BLACK_SHORT] = true;
	if (castle.find('q') != std::string::npos)
		board.castle[BLACK_LONG] = true;

	std::string ep = splitted[3];
	if (ep.compare("-")) {
		for (int i = 0; i < 64; i++) {
			if (ep == squares[i]) {
				board.enPassant = i;
				break;
			}
		}
	}
	else
		board.enPassant = -1;

	std::vector<std::string> line = split(splitted[0], '/');

	for (int i = 0; i < 8; i++) {
		int cur = 0;
		std::string cur_rank = line[i];
		for (size_t j = 0; j < cur_rank.size(); j++) {
			if (isdigit(cur_rank[j])) {
				cur += cur_rank[j] - '0';
				continue;
			}

			SET1(board.allPieces, 63 - (i * 8 + cur));

			int piece = bitboard::pieces.find(cur_rank[j]);
			board.mailbox[63 - (i * 8 + cur)] = piece;
			if (piece >= 6)
				SET1(board.pieces[BLACK][piece - 6], 63 - (i * 8 + cur));
			else
				SET1(board.pieces[WHITE][piece], 63 - (i * 8 + cur));

			cur++;
		}
	}

	for (int i = 0; i < 6; i++) {
		board.pieces[WHITE][ALL] |= board.pieces[WHITE][i];
		board.pieces[BLACK][ALL] |= board.pieces[BLACK][i];
	}
}
std::string bitboard::encode(const bitboard::Position &board) {
	std::string fen = "";
	for (int i = 0; i < 8; i++) {
		int empty = 0;
		for (int j = 0; j < 8; j++) {
			int index = 63 - (i * 8 + j);
			if (QUERY(board.allPieces, index)) {
				if (empty != 0)
					fen += empty + '0';
				empty = 0;
				for (int k = 0; k < 6; k++) {
					if (QUERY(board.pieces[WHITE][k], index)) {
						fen += bitboard::pieces[k];
						break;
					}
				}
				for (int k = 0; k < 6; k++) {
					if (QUERY(board.pieces[BLACK][k], index)) {
						fen += bitboard::pieces[k + 6];
						break;
					}
				}
			}
			else
				empty++;
		}

		if (empty != 0)
			fen += empty + '0';

		if (i < 7)
			fen += "/";
	}
	fen += " ";

	fen += board.turn ? 'b' : 'w';
	fen += " ";

	std::string castle_rights = "";
	if (board.castle[WHITE_SHORT])
		castle_rights += "K";
	if (board.castle[WHITE_LONG])
		castle_rights += "Q";
	if (board.castle[BLACK_SHORT])
		castle_rights += "k";
	if (board.castle[BLACK_LONG])
		castle_rights += "q";
	if (castle_rights == "")
		castle_rights = "-";

	fen += castle_rights + " ";

	if (board.enPassant == -1)
		fen += "-";
	else
		fen += bitboard::squares[board.enPassant];
		
	fen += " " + std::to_string(board.fiftyMoveClock);
	fen += " " + std::to_string(board.fullMove);

	return fen;
}

void bitboard::print_board(const bitboard::Position &board) {
	std::cout << "╭───┬───┬───┬───┬───┬───┬───┬───╮" << "\n";

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			int index = 63 - (i * 8 + j);
			// if (QUERY(board.all_pieces, index)) {
			// 	for (int k = 0; k < 6; k++) {
			// 		if (QUERY(board.pieces[0][k], index)) {
			// 			std::cout << "│ " << bitboard::pieces[k] << " ";
			// 			break;
			// 		}
			// 	}
			// 	for (int k = 0; k < 6; k++) {
			// 		if (QUERY(board.pieces[1][k], index)) {
			// 			std::cout << "│ " << bitboard::pieces[k + 6] << " ";
			// 			break;
			// 		}
			// 	}
			// }
			// else
			// 	std::cout << "│   ";
			if (board.mailbox[index] != -1)
				std::cout << "│ " << bitboard::pieces[board.mailbox[index]] << " ";
			else
				std::cout << "│   ";
		}

		std::cout << "│";

		if (i < 7)
			std::cout << "\n├───┼───┼───┼───┼───┼───┼───┼───┤\n";
		else
			std::cout << "\n╰───┴───┴───┴───┴───┴───┴───┴───╯\n";
	}

	std::cout << "\nFEN: " << encode(board);
	std::cout << "\nWhite Kingside: " << board.castle[WHITE_SHORT];
	std::cout << "    White Queenside: " << board.castle[WHITE_LONG];
	std::cout << "\nBlack Kingside: " << board.castle[BLACK_SHORT];
	std::cout << "    Black Queenside: " << board.castle[BLACK_LONG];

	std::cout << "\nEn Passant Square: " << (board.enPassant == -1 ? "None" : bitboard::squares[board.enPassant]);
	std::cout << "\nTurn: " << (board.turn ? "Black" : "White");

	std::cout << "\n";
}
