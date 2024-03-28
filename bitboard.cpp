#include <iostream>
#include <string>
#include <cstring>
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
std::unordered_set<ull> bitboard::prevPositions;

std::vector<std::string> split(std::string str, char split_on) {
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

void bitboard::Position::decode(std::string fen) {
	for (int i = 0; i < 64; i++)
		this->mailbox[i] = -1;
	this->allPieces = 0ull;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 7; j++)
			this->pieces[i][j] = 0ull;
	}
	for (int i = 0; i < 4; i++)
		this->castle[i] = false;
	this->turn = 0;
	this->enPassant = -1;
	this->fiftyMoveClock = 0;
	this->fullMove = 0;

	std::vector<std::string> splitted = split(fen, ' ');

	if (!splitted[1].compare("w"))
		this->turn = WHITE;
	else
		this->turn = BLACK;

	try {
		this->fiftyMoveClock = stoi(splitted[4]);
		this->fullMove = stoi(splitted[5]);
	}
	catch (...) { }


	std::string castle = splitted[2];
	if (castle.find('K') != std::string::npos)
		this->castle[WHITE_SHORT] = true;
	if (castle.find('Q') != std::string::npos)
		this->castle[WHITE_LONG] = true;
	if (castle.find('k') != std::string::npos)
		this->castle[BLACK_SHORT] = true;
	if (castle.find('q') != std::string::npos)
		this->castle[BLACK_LONG] = true;

	std::string ep = splitted[3];
	if (ep.compare("-")) {
		for (int i = 0; i < 64; i++) {
			if (ep == squares[i]) {
				this->enPassant = i;
				break;
			}
		}
	}
	else
		this->enPassant = -1;

	std::vector<std::string> line = split(splitted[0], '/');

	for (int i = 0; i < 8; i++) {
		int cur = 0;
		std::string cur_rank = line[i];
		for (size_t j = 0; j < cur_rank.size(); j++) {
			if (isdigit(cur_rank[j])) {
				cur += cur_rank[j] - '0';
				continue;
			}

			SET1(this->allPieces, 63 - (i * 8 + cur));

			int piece = bitboard::pieces.find(cur_rank[j]);
			this->mailbox[63 - (i * 8 + cur)] = piece;
			if (piece >= 6)
				SET1(this->pieces[BLACK][piece - 6], 63 - (i * 8 + cur));
			else
				SET1(this->pieces[WHITE][piece], 63 - (i * 8 + cur));

			cur++;
		}
	}

	for (int i = 0; i < 6; i++) {
		this->pieces[WHITE][ALL] |= this->pieces[WHITE][i];
		this->pieces[BLACK][ALL] |= this->pieces[BLACK][i];
	}
}
bitboard::Position::Position(std::string fen) {
	decode(fen);	
}

bitboard::Position &bitboard::Position::operator=(const Position &src) {
	this->allPieces = src.allPieces;

	this->pieces = src.pieces;
	this->mailbox = src.mailbox;
	// memcpy(this->pieces, src.pieces, sizeof(this->pieces));
	// memcpy(this->mailbox, src.mailbox, sizeof(this->mailbox));

	this->turn = src.turn;
	this->enPassant = src.enPassant;
	this->castle = src.castle;
	// memcpy(this->castle, src.castle, sizeof(src.castle));

	this->fiftyMoveClock = src.fiftyMoveClock;
	this->fullMove = src.fullMove;

	return *this;
}
bitboard::Position::Position(const Position &src) {
	this->operator=(src);
}

std::string bitboard::Position::encode() const {
	std::string fen = "";
	for (int i = 0; i < 8; i++) {
		int empty = 0;
		for (int j = 0; j < 8; j++) {
			int index = 63 - (i * 8 + j);
			if (QUERY(this->allPieces, index)) {
				if (empty != 0)
					fen += empty + '0';
				empty = 0;
				for (int k = 0; k < 6; k++) {
					if (QUERY(this->pieces[WHITE][k], index)) {
						fen += bitboard::pieces[k];
						break;
					}
				}
				for (int k = 0; k < 6; k++) {
					if (QUERY(this->pieces[BLACK][k], index)) {
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

	fen += this->turn ? 'b' : 'w';
	fen += " ";

	std::string castle_rights = "";
	if (this->castle[WHITE_SHORT])
		castle_rights += "K";
	if (this->castle[WHITE_LONG])
		castle_rights += "Q";
	if (this->castle[BLACK_SHORT])
		castle_rights += "k";
	if (this->castle[BLACK_LONG])
		castle_rights += "q";
	if (castle_rights == "")
		castle_rights = "-";

	fen += castle_rights + " ";

	if (this->enPassant == -1)
		fen += "-";
	else
		fen += bitboard::squares[this->enPassant];
		
	fen += " " + std::to_string(this->fiftyMoveClock);
	fen += " " + std::to_string(this->fullMove);

	return fen;
}
void bitboard::Position::print() const {
	std::cout << "╭───┬───┬───┬───┬───┬───┬───┬───╮" << "\n";

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			int index = 63 - (i * 8 + j);
			if (this->mailbox[index] != -1)
				std::cout << "│ " << bitboard::pieces[this->mailbox[index]] << " ";
			else
				std::cout << "│   ";
		}

		std::cout << "│";

		if (i < 7)
			std::cout << "\n├───┼───┼───┼───┼───┼───┼───┼───┤\n";
		else
			std::cout << "\n╰───┴───┴───┴───┴───┴───┴───┴───╯\n";
	}

	std::cout << "\nFEN: " << this->encode();
	std::cout << "\nWhite Kingside: " << this->castle[WHITE_SHORT];
	std::cout << "    White Queenside: " << this->castle[WHITE_LONG];
	std::cout << "\nBlack Kingside: " << this->castle[BLACK_SHORT];
	std::cout << "    Black Queenside: " << this->castle[BLACK_LONG];

	std::cout << "\nEn Passant Square: " << (this->enPassant == -1 ? "None" : bitboard::squares[this->enPassant]);
	std::cout << "\nTurn: " << (this->turn ? "Black" : "White");

	std::cout << "\n";
}