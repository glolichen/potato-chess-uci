#include <iostream>
#include <sstream>
#include <string>

#include "bitboard.h"
#include "move.h"
#include "hash.h"

std::string move::to_string(int move) {
	std::stringstream ss;

	ss << bitboard::squares[SOURCE(move)];

	int dest = DEST(move);
	ss << bitboard::squares[dest];

	int promote = PROMOTE(move);
	if (promote)
		ss << bitboard::pieces[PROMOTE(move) + 6];

	return ss.str();
}
int move::uci_to_move(bitboard::Position &board, std::string uci) {
	int from = (uci[1] - '0' - 1) * 8 + (7 - (uci[0] - 'a'));
	int dest = (uci[3] - '0' - 1) * 8 + (7 - (uci[2] - 'a'));

	int promote = uci.length() == 5 ? uci[4] : 0;
	return move::uci_to_move(board, from, dest, promote);
}

int move::uci_to_move(bitboard::Position &board, int from, int dest, int promote) {
	if (promote != 0) {
		switch (promote) {
			case 'q':
				promote = QUEEN;
				break;
			case 'r':
				promote = ROOK;
				break;
			case 'b':
				promote = BISHOP;
				break;
			case 'n':
				promote = KNIGHT;
				break;
		}
		return NEW_MOVE(from, dest, 0, promote, 0);
	}

	int castle = 0;
	bool e1IsKing = board.mailbox[E1] == KING || board.mailbox[E1] == KING + 6;
	bool e8IsKing = board.mailbox[E8] == KING || board.mailbox[E8] == KING + 6;
	if ((from == E1 && dest == G1 && e1IsKing) || (from == E8 && dest == G8 && e8IsKing))
		castle = 1;
	else if ((from == E1 && dest == C1 && e1IsKing) || (from == E8 && dest == C8 && e8IsKing))
		castle = 2;

	int ep = 0;
	if (std::abs(dest - from) == 7 || std::abs(dest - from) == 9) {
		if (board.mailbox[from] == PAWN || board.mailbox[from] == PAWN + 6) {
			if (board.mailbox[dest] == -1)
				ep = 1;
		}
	}

	return NEW_MOVE(from, dest, castle, 0, ep);
}

void move::make_move(bitboard::Position &board, int move) {
	int source = SOURCE(move);
	int dest = DEST(move);
	int promote = PROMOTE(move);
	board.enPassant = -1;

	if (IS_EP(move)) {
		int captured = dest + (board.turn ? 8 : -8);
		SET0(board.allPieces, captured);
		if (board.turn) {
			SET0(board.pieces[WHITE][PAWN], captured);
			SET0(board.pieces[WHITE][ALL], captured);
		}
		else {
			SET0(board.pieces[BLACK][PAWN], captured);
			SET0(board.pieces[BLACK][ALL], captured);
		}

		board.mailbox[captured] = -1;
	}

	if (board.turn && board.mailbox[source] == PAWN + 6 && source - dest == 16) {
		if (board.mailbox[dest - 1] == PAWN || board.mailbox[dest + 1] == PAWN)
			board.enPassant = dest + 8;
	}
	else if (!board.turn && board.mailbox[source] == PAWN && dest - source == 16) {
		if (board.mailbox[dest - 1] == PAWN + 6 || board.mailbox[dest + 1] == PAWN + 6)
			board.enPassant = dest - 8;
	}

	if (promote) {
		SET1(board.allPieces, dest);
		SET0(board.allPieces, source);
		if (board.turn == BLACK) {
			if (board.mailbox[dest] != -1) {
				SET0(board.pieces[WHITE][board.mailbox[dest]], dest);
				SET0(board.pieces[WHITE][ALL], dest);
			}
			SET1(board.pieces[BLACK][promote], dest);
			SET0(board.pieces[BLACK][PAWN], source);
			SET1(board.pieces[BLACK][ALL], dest);
			SET0(board.pieces[BLACK][ALL], source);
			promote += 6;
		}
		else {
			if (board.mailbox[dest] != -1) {
				SET0(board.pieces[BLACK][board.mailbox[dest] - 6], dest);
				SET0(board.pieces[BLACK][ALL], dest);
			}
			SET1(board.pieces[WHITE][promote], dest);
			SET0(board.pieces[WHITE][PAWN], source);
			SET1(board.pieces[WHITE][ALL], dest);
			SET0(board.pieces[WHITE][ALL], source);
		}

		if (dest == 0 && board.mailbox[dest] == ROOK)
			board.castle[WHITE_SHORT] = false;
		else if (dest == 7 && board.mailbox[dest] == ROOK)
			board.castle[WHITE_LONG] = false;
		else if (dest == 56 && board.mailbox[dest] == ROOK + 6)
			board.castle[BLACK_SHORT] = false;
		else if (dest == 63 && board.mailbox[dest] == ROOK + 6)
			board.castle[BLACK_LONG] = false;

		board.mailbox[dest] = promote;
		board.mailbox[source] = -1;
	}
	else {
		SET1(board.allPieces, dest);
		SET0(board.allPieces, source);
		if (board.turn) {
			if (board.mailbox[dest] != -1) {
				SET0(board.pieces[WHITE][board.mailbox[dest]], dest);
				SET0(board.pieces[WHITE][ALL], dest);
			}
			SET1(board.pieces[BLACK][board.mailbox[source] - 6], dest);
			SET0(board.pieces[BLACK][board.mailbox[source] - 6], source);
			SET1(board.pieces[BLACK][ALL], dest);
			SET0(board.pieces[BLACK][ALL], source);
		}
		else {
			if (board.mailbox[dest] != -1) {
				SET0(board.pieces[BLACK][board.mailbox[dest] - 6], dest);
				SET0(board.pieces[BLACK][ALL], dest);
			}
			SET1(board.pieces[WHITE][board.mailbox[source]], dest);
			SET0(board.pieces[WHITE][board.mailbox[source]], source);
			SET1(board.pieces[WHITE][ALL], dest);
			SET0(board.pieces[WHITE][ALL], source);
		}

		if (board.mailbox[source] == KING || board.mailbox[source] == KING + 6) {
			board.castle[0 + board.turn * 2] = false;
			board.castle[1 + board.turn * 2] = false;
		}
		else if (source == 0 && board.mailbox[source] == ROOK)
			board.castle[WHITE_SHORT] = false;
		else if (source == 7 && board.mailbox[source] == ROOK)
			board.castle[WHITE_LONG] = false;
		else if (source == 56 && board.mailbox[source] == ROOK + 6)
			board.castle[BLACK_SHORT] = false;
		else if (source == 63 && board.mailbox[source] == ROOK + 6)
			board.castle[BLACK_LONG] = false;
		else if (dest == 0 && board.mailbox[dest] == ROOK)
			board.castle[WHITE_SHORT] = false;
		else if (dest == 7 && board.mailbox[dest] == ROOK)
			board.castle[WHITE_LONG] = false;
		else if (dest == 56 && board.mailbox[dest] == ROOK + 6)
			board.castle[BLACK_SHORT] = false;
		else if (dest == 63 && board.mailbox[dest] == ROOK + 6)
			board.castle[BLACK_LONG] = false;

		int castle = CASTLE(move);
		if (castle) {
			int rookSource, rookDest;
			if (board.turn) {
				rookSource = castle == 1 ? 56 : 63;
				rookDest = castle == 1 ? 58 : 60;
				SET1(board.pieces[BLACK][ROOK], rookDest);
				SET0(board.pieces[BLACK][ROOK], rookSource);
				SET1(board.pieces[BLACK][ALL], rookDest);
				SET0(board.pieces[BLACK][ALL], rookSource);
			}
			else {
				rookSource = castle == 1 ? 0 : 7;
				rookDest = castle == 1 ? 2 : 4;
				SET1(board.pieces[WHITE][ROOK], rookDest);
				SET0(board.pieces[WHITE][ROOK], rookSource);
				SET1(board.pieces[WHITE][ALL], rookDest);
				SET0(board.pieces[WHITE][ALL], rookSource);
			}

			SET1(board.allPieces, rookDest);
			SET0(board.allPieces, rookSource);

			board.mailbox[rookDest] = ROOK + (board.turn ? 6 : 0);
			board.mailbox[rookSource] = -1;
		}

		board.mailbox[dest] = board.mailbox[source];
		board.mailbox[source] = -1;
	}

	board.turn = !board.turn;

	// hashdefs::ZobristTuple hashes = hash::hash(board);
	// board.repetition[hashes] = (*board.repetition.find(hashes)).second + 1;
}