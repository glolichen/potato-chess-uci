#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>

#include "bitboard.h"
#include "maps.h"
#include "movegen.h"
#include "move.h"

ull get_pawn_moves(const bitboard::Position &board, int square) {
	int multiplier = board.turn ? -1 : 1;
	if (QUERY(board.allPieces, square + 8 * multiplier))
		return 0ull;
	ull moves = (1ull << (square + 8 * multiplier));
	if ((square / 8 == 1 && !board.turn) || (square / 8 == 6 && board.turn))
		moves |= (1ull << (square + 16 * multiplier));

	ull answer = moves & ~board.allPieces;
	return answer;
}

ull movegen::get_pawn_attacks(int square, bool color) {
	ull attacks = 0;
	if (color) {
		if (square % 8 > 0)
			SET1(attacks, square - 9);
		if (square % 8 < 7)
			SET1(attacks, square - 7);
	}
	else {
		if (square % 8 > 0)
			SET1(attacks, square + 7);
		if (square % 8 < 7)
			SET1(attacks, square + 9);
	}
	return attacks;
}
ull movegen::get_knight_attacks(int square) {
	return maps::knight[square];
}
ull movegen::get_bishop_attacks(const bitboard::Position &board, const ull &pieces_no_king, int square) {
	ull attacks, blockers;

	blockers = maps::bishop[square][0] & pieces_no_king;
	if (blockers)
		attacks = ~maps::bishop[__builtin_ctzll(blockers)][0] & maps::bishop[square][0];
	else
		attacks = maps::bishop[square][0];
	
	blockers = maps::bishop[square][1] & pieces_no_king;
	if (blockers)
		attacks |= ~maps::bishop[63 - __builtin_clzll(blockers)][1] & maps::bishop[square][1];
	else
		attacks |= maps::bishop[square][1];

	blockers = maps::bishop[square][2] & pieces_no_king;
	if (blockers)
		attacks |= ~maps::bishop[63 - __builtin_clzll(blockers)][2] & maps::bishop[square][2];
	else
		attacks |= maps::bishop[square][2];

	blockers = maps::bishop[square][3] & pieces_no_king;
	if (blockers)
		attacks |= ~maps::bishop[__builtin_ctzll(blockers)][3] & maps::bishop[square][3];
	else
		attacks |= maps::bishop[square][3];

	return attacks;
}
ull movegen::get_rook_attacks(const bitboard::Position &board, const ull &pieces_no_king, int square) {
	ull attacks, blockers;
	
	blockers = maps::rook[square][0] & pieces_no_king;
	if (blockers)
		attacks = ~maps::rook[__builtin_ctzll(blockers)][0] & maps::rook[square][0];
	else
		attacks = maps::rook[square][0];
	
	blockers = maps::rook[square][1] & pieces_no_king;
	if (blockers)
		attacks |= ~maps::rook[63 - __builtin_clzll(blockers)][1] & maps::rook[square][1];
	else
		attacks |= maps::rook[square][1];

	blockers = maps::rook[square][2] & pieces_no_king;
	if (blockers)
		attacks |= ~maps::rook[63 - __builtin_clzll(blockers)][2] & maps::rook[square][2];
	else
		attacks |= maps::rook[square][2];

	blockers = maps::rook[square][3] & pieces_no_king;
	if (blockers)
		attacks |= ~maps::rook[__builtin_ctzll(blockers)][3] & maps::rook[square][3];
	else
		attacks |= maps::rook[square][3];

	return attacks;
}
ull movegen::get_queen_attacks(const bitboard::Position &board, const ull &pieces_no_king, int square) {
	return get_bishop_attacks(board, pieces_no_king, square) | get_rook_attacks(board, pieces_no_king, square);
}
ull get_king_attacks(int square) {
	ull attacks = 0;

	bool topEdge = square / 8 > 0;
	bool rightEdge = square % 8 < 7;
	bool bottomEdge = square / 8 < 7;
	bool leftEdge = square % 8 > 0;

	if (topEdge)
		SET1(attacks, square - 8);
	if (rightEdge)
		SET1(attacks, square + 1);
	if (bottomEdge)
		SET1(attacks, square + 8);
	if (leftEdge)
		SET1(attacks, square - 1);
	
	if (topEdge && rightEdge)
		SET1(attacks, square - 7);
	if (bottomEdge && rightEdge)
		SET1(attacks, square + 9);
	if (bottomEdge && leftEdge)
		SET1(attacks, square + 7);
	if (topEdge && leftEdge)
		SET1(attacks, square - 9);
	
	return attacks;
} 

ull getAttacked(const bitboard::Position &board, bool color) {
	ull piecesNoKing = board.allPieces ^ board.pieces[color][KING];
	color = !color;
	ull pieces, attacks = 0;

	pieces = board.pieces[color][PAWN];
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		attacks |= movegen::get_pawn_attacks(pos, color);
		SET0(pieces, pos);
	}

	pieces = board.pieces[color][KNIGHT];
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		attacks |= movegen::get_knight_attacks(pos);
		SET0(pieces, pos);
	}
	
	pieces = board.pieces[color][BISHOP];
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		attacks |= movegen::get_bishop_attacks(board, piecesNoKing, pos);
		SET0(pieces, pos);
	}

	pieces = board.pieces[color][ROOK];
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		attacks |= movegen::get_rook_attacks(board, piecesNoKing, pos);
		SET0(pieces, pos);
	}
	
	pieces = board.pieces[color][QUEEN];
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		attacks |= movegen::get_queen_attacks(board, piecesNoKing, pos);
		SET0(pieces, pos);
	}
	
	pieces = board.pieces[color][KING];
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		attacks |= get_king_attacks(pos);
		SET0(pieces, pos);
	}

	return attacks;
}
ull movegen::get_checks(const bitboard::Position &board, bool color) {
	int kingPos = __builtin_ctzll(board.pieces[color][KING]);

	ull checks = get_pawn_attacks(kingPos, color) & board.pieces[!color][PAWN];
	checks |= get_knight_attacks(kingPos) & board.pieces[!color][KNIGHT];
	checks |= get_bishop_attacks(board, board.allPieces, kingPos) & (board.pieces[!color][BISHOP] | board.pieces[!color][QUEEN]);
	checks |= get_rook_attacks(board, board.allPieces, kingPos) & (board.pieces[!color][ROOK] | board.pieces[!color][QUEEN]);
	
	return checks;
}
ull getPinned(const bitboard::Position &board, bool color) {
	int pinnedPiece, kingPos = __builtin_ctzll(board.pieces[color][KING]);;
	ull blockers, pinned = 0;
	ull ownPieces = board.pieces[color][ALL];
	ull rookQueen = board.pieces[!color][ROOK] | board.pieces[!color][QUEEN];
	ull bishopQueen = board.pieces[!color][BISHOP] | board.pieces[!color][QUEEN];
	
	blockers = maps::rook[kingPos][0] & board.allPieces;
	if (blockers) {
		pinnedPiece = __builtin_ctzll(blockers);
		SET0(blockers, pinnedPiece);
		if (blockers) {
			if (QUERY(rookQueen, 63 - __builtin_clzll(~maps::rook[__builtin_ctzll(blockers)][0] & maps::rook[kingPos][0]))) {
				ull potential = 1ull << pinnedPiece;
				if (potential & ownPieces)
					pinned |= potential;
			}
		}
	}
	
	blockers = maps::rook[kingPos][1] & board.allPieces;
	if (blockers) {
		pinnedPiece = 63 - __builtin_clzll(blockers);
		SET0(blockers, pinnedPiece);
		if (blockers) {
			if (QUERY(rookQueen, __builtin_ctzll(~maps::rook[63 - __builtin_clzll(blockers)][1] & maps::rook[kingPos][1]))) {
				ull potential = 1ull << pinnedPiece;
				if (potential & ownPieces)
					pinned |= potential;
			}
		}
	}
	
	blockers = maps::rook[kingPos][2] & board.allPieces;
	if (blockers) {
		pinnedPiece = 63 - __builtin_clzll(blockers);
		SET0(blockers, pinnedPiece);
		if (blockers) {
			if (QUERY(rookQueen, __builtin_ctzll(~maps::rook[63 - __builtin_clzll(blockers)][2] & maps::rook[kingPos][2]))) {
				ull potential = 1ull << pinnedPiece;
				if (potential & ownPieces)
					pinned |= potential;
			}
		}
	}
	
	blockers = maps::rook[kingPos][3] & board.allPieces;
	if (blockers) {
		pinnedPiece = __builtin_ctzll(blockers);
		SET0(blockers, pinnedPiece);
		if (blockers) {
			if (QUERY(rookQueen, 63 - __builtin_clzll(~maps::rook[__builtin_ctzll(blockers)][3] & maps::rook[kingPos][3]))) {
				ull potential = 1ull << pinnedPiece;
				if (potential & ownPieces)
					pinned |= potential;
			}
		}
	}
	
	blockers = maps::bishop[kingPos][0] & board.allPieces;
	if (blockers) {
		pinnedPiece = __builtin_ctzll(blockers);
		SET0(blockers, pinnedPiece);
		if (blockers) {
			if (QUERY(bishopQueen, 63 - __builtin_clzll(~maps::bishop[__builtin_ctzll(blockers)][0] & maps::bishop[kingPos][0]))) {
				ull potential = 1ull << pinnedPiece;
				if (potential & ownPieces)
					pinned |= potential;
			}
		}
	}
	
	blockers = maps::bishop[kingPos][1] & board.allPieces;
	if (blockers) {
		pinnedPiece = 63 - __builtin_clzll(blockers);
		SET0(blockers, pinnedPiece);
		if (blockers) {
			if (QUERY(bishopQueen, __builtin_ctzll(~maps::bishop[63 - __builtin_clzll(blockers)][1] & maps::bishop[kingPos][1]))) {
				ull potential = 1ull << pinnedPiece;
				if (potential & ownPieces)
					pinned |= potential;
			}
		}
	}
	
	blockers = maps::bishop[kingPos][2] & board.allPieces;
	if (blockers) {
		pinnedPiece = 63 - __builtin_clzll(blockers);
		SET0(blockers, pinnedPiece);
		if (blockers) {
			if (QUERY(bishopQueen, __builtin_ctzll(~maps::bishop[63 - __builtin_clzll(blockers)][2] & maps::bishop[kingPos][2]))) {
				ull potential = 1ull << pinnedPiece;
				if (potential & ownPieces)
					pinned |= potential;
			}
		}
	}
	
	blockers = maps::bishop[kingPos][3] & board.allPieces;
	if (blockers) {
		pinnedPiece = __builtin_ctzll(blockers);
		SET0(blockers, pinnedPiece);
		if (blockers) {
			if (QUERY(bishopQueen, 63 - __builtin_clzll(~maps::bishop[__builtin_ctzll(blockers)][3] & maps::bishop[kingPos][3]))) {
				ull potential = 1ull << pinnedPiece;
				if (potential & ownPieces)
					pinned |= potential;
			}
		}
	}

	return pinned;
}

void movegen::move_gen(const bitboard::Position &board, std::vector<int> &moves) {
	moves.clear();

	ull attacked = getAttacked(board, board.turn);
	ull checks = get_checks(board, board.turn);
	ull pinned = getPinned(board, board.turn);
	ull pieces, blocks;

	int kingPos = __builtin_ctzll(board.pieces[board.turn][KING]);
	if (board.enPassant != -1) {
		int dir = board.turn ? 1 : -1;
		int center = board.enPassant + (dir * 8);
		int ownPawn = PAWN + (board.turn ? 6 : 0);

		if (center % 8 > 0 && board.mailbox[center - 1] == ownPawn) {
			bool ok = true;
			if (QUERY(pinned, center - 1)) {
				if ((kingPos - (center - 1)) % 8 == 0)
					ok = false;
				else if ((kingPos - (center - 1)) % (board.enPassant - (center - 1)) != 0)
					ok = false;
			}
			else {
				ull left = board.allPieces & (maps::rook[center - 1][3] ^ (1 << center));
				ull right = board.allPieces & (maps::rook[center][1] ^ (1 << (center - 1)));
				int leftPiece = __builtin_ctzll(left);
				int rightPiece = __builtin_ctzll(right);

				if (left == 1ull << leftPiece && right == 1ull << rightPiece) {
					int oppRook = ROOK + (board.turn ? 0 : 6);
					int oppQueen = QUEEN + (board.turn ? 0 : 6);
					if (leftPiece == kingPos && (board.mailbox[rightPiece] == oppRook || board.mailbox[rightPiece] == oppQueen))
						ok = false;
					else if (rightPiece == kingPos && (board.mailbox[leftPiece] == oppRook || board.mailbox[leftPiece] == oppQueen))
						ok = false;
				}
			}
			if (ok)
				moves.push_back(NEW_MOVE(center - 1, board.enPassant, 0, 0, 1));
		}
		if (center % 8 < 7 && board.mailbox[center + 1] == ownPawn) {
			bool ok = true;
			if (QUERY(pinned, center + 1)) {
				if ((kingPos - (center + 1)) % 8 == 0)
					ok = false;
				else if ((kingPos - (center + 1)) % (board.enPassant - (center + 1)) != 0)
					ok = false;
			}
			else {
				ull left = board.allPieces & (maps::rook[center + 1][3] ^ (1 << (center + 2)));
				ull right = board.allPieces & (maps::rook[center][1] ^ (1 << (center - 1)));
				int leftPiece = __builtin_ctzll(left);
				int rightPiece = __builtin_ctzll(right);

				if (left == 1ull << leftPiece && right == 1ull << rightPiece) {
					int oppRook = ROOK + (board.turn ? 0 : 6);
					int oppQueen = QUEEN + (board.turn ? 0 : 6);
					if (leftPiece == kingPos && (board.mailbox[rightPiece] == oppRook || board.mailbox[rightPiece] == oppQueen))
						ok = false;
					else if (rightPiece == kingPos && (board.mailbox[leftPiece] == oppRook || board.mailbox[leftPiece] == oppQueen))
						ok = false;
				}
			}
			if (ok)
				moves.push_back(NEW_MOVE(center + 1, board.enPassant, 0, 0, 1));
		}

		// if (board.mailbox[center - 1] == pawn && center % 8 > 0) {
		// 	int move = NEW_MOVE(center - 1, board.enPassant, 0, 0, 1);

		// 	bitboard::Position newBoard;
		// 	memcpy(&newBoard, &board, sizeof(board));

		// 	move::make_move(newBoard, move);
		// 	if (!get_checks(newBoard, !newBoard.turn))
		// 		moves.push_back(move);
		// }
		// if (board.mailbox[center + 1] == pawn && center % 8 < 7) {
		// 	int move = NEW_MOVE(center + 1, board.enPassant, 0, 0, 1);

		// 	bitboard::Position new_board;
		// 	memcpy(&new_board, &board, sizeof(board));

		// 	move::make_move(new_board, move);
		// 	if (!get_checks(new_board, !new_board.turn))
		// 		moves.push_back(move);
	}

	if (checks) {
		if (checks & (checks - 1)) { // double check
			ull attacks = get_king_attacks(kingPos) & ~attacked & ~board.pieces[board.turn][ALL];
			while (attacks) {
				int pos = __builtin_ctzll(attacks);
				moves.push_back(NEW_MOVE(kingPos, pos, 0, 0, 0));
				SET0(attacks, pos);
			}
			return;
		}
		
		int checkerPos = __builtin_ctzll(checks);
		int piece = board.mailbox[checkerPos];
		if (piece >= 6)
			piece -= 6;

		if (board.mailbox[checkerPos] == KNIGHT)
			blocks = 1ull << checkerPos;
		else {
			ull filled = (1ull << checkerPos) | (1ull << kingPos);
			filled = maps::fill[__builtin_ctzll(filled)][63 - __builtin_clzll(filled)];

			if (maps::pinnedOffsets[piece][kingPos].count(kingPos - checkerPos))
				blocks = maps::pinnedOffsets[piece][kingPos].at(kingPos - checkerPos) & filled & ~board.pieces[board.turn][KING];
			else
				blocks = 1ull << checkerPos;
		}
	}
	else
		blocks = ~0ull;

	pieces = board.pieces[board.turn][PAWN] & ~pinned;
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		ull attacks = ((get_pawn_attacks(pos, board.turn) & board.pieces[!board.turn][ALL]) | 
			get_pawn_moves(board, pos)) & blocks;
		while (attacks) {
			int dest = __builtin_ctzll(attacks);
			switch (dest / 8) {
				case 0:
				case 7:
					moves.push_back(NEW_MOVE(pos, dest, 0, QUEEN, 0));
					moves.push_back(NEW_MOVE(pos, dest, 0, ROOK, 0));
					moves.push_back(NEW_MOVE(pos, dest, 0, BISHOP, 0));
					moves.push_back(NEW_MOVE(pos, dest, 0, KNIGHT, 0));
					break;
				default:
					moves.push_back(NEW_MOVE(pos, dest, 0, 0, 0));
			}
			SET0(attacks, dest);
		}
		SET0(pieces, pos);
	}

	pieces = board.pieces[board.turn][PAWN] & pinned;
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		ull attacks = ((get_pawn_attacks(pos, board.turn) & board.pieces[!board.turn][ALL]) | 
			get_pawn_moves(board, pos)) & blocks;
		attacks &= maps::pinnedOffsetsAll[kingPos].at(kingPos - pos);
		while (attacks) {
			int dest = __builtin_ctzll(attacks);
			switch (dest / 8) {
				case 0:
				case 7:
					moves.push_back(NEW_MOVE(pos, dest, 0, QUEEN, 0));
					moves.push_back(NEW_MOVE(pos, dest, 0, ROOK, 0));
					moves.push_back(NEW_MOVE(pos, dest, 0, BISHOP, 0));
					moves.push_back(NEW_MOVE(pos, dest, 0, KNIGHT, 0));
					break;
				default:
					moves.push_back(NEW_MOVE(pos, dest, 0, 0, 0));
			}
			SET0(attacks, dest);
		}
		SET0(pieces, pos);
	}

	pieces = board.pieces[board.turn][KNIGHT] & ~pinned;
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		ull attacks = get_knight_attacks(pos) & ~board.pieces[board.turn][ALL] & blocks;
		while (attacks) {
			int dest = __builtin_ctzll(attacks);
			moves.push_back(NEW_MOVE(pos, dest, 0, 0, 0));
			SET0(attacks, dest);
		}
		SET0(pieces, pos);
	}

	pieces = board.pieces[board.turn][BISHOP] & ~pinned;
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		ull attacks = get_bishop_attacks(board, board.allPieces, pos) & ~board.pieces[board.turn][ALL] & blocks;
		while (attacks) {
			int dest = __builtin_ctzll(attacks);
			moves.push_back(NEW_MOVE(pos, dest, 0, 0, 0));
			SET0(attacks, dest);
		}
		SET0(pieces, pos);
	}

	pieces = board.pieces[board.turn][BISHOP] & pinned;
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		ull attacks = get_bishop_attacks(board, board.allPieces, pos) & maps::pinnedOffsetsAll[pos].at(kingPos - pos) & ~board.pieces[board.turn][ALL] & blocks;
		while (attacks) {
			int dest = __builtin_ctzll(attacks);
			moves.push_back(NEW_MOVE(pos, dest, 0, 0, 0));
			SET0(attacks, dest);
		}
		SET0(pieces, pos);
	}

	pieces = board.pieces[board.turn][ROOK] & ~pinned;
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		ull attacks = get_rook_attacks(board, board.allPieces, pos) & ~board.pieces[board.turn][ALL] & blocks;
		while (attacks) {
			int dest = __builtin_ctzll(attacks);
			moves.push_back(NEW_MOVE(pos, dest, 0, 0, 0));
			SET0(attacks, dest);
		}
		SET0(pieces, pos);
	}

	pieces = board.pieces[board.turn][ROOK] & pinned;
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		ull attacks = get_rook_attacks(board, board.allPieces, pos) & maps::pinnedOffsetsAll[pos].at(kingPos - pos) & ~board.pieces[board.turn][ALL] & blocks;
		while (attacks) {
			int dest = __builtin_ctzll(attacks);
			moves.push_back(NEW_MOVE(pos, dest, 0, 0, 0));
			SET0(attacks, dest);
		}
		SET0(pieces, pos);
	}

	pieces = board.pieces[board.turn][QUEEN] & ~pinned;
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		ull attacks = get_queen_attacks(board, board.allPieces, pos) & ~board.pieces[board.turn][ALL] & blocks;
		while (attacks) {
			int dest = __builtin_ctzll(attacks);
			moves.push_back(NEW_MOVE(pos, dest, 0, 0, 0));
			SET0(attacks, dest);
		}
		SET0(pieces, pos);
	}

	pieces = board.pieces[board.turn][QUEEN] & pinned;
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		ull attacks = get_queen_attacks(board, board.allPieces, pos) & maps::pinnedOffsetsAll[pos].at(kingPos - pos) & ~board.pieces[board.turn][ALL] & blocks;
		while (attacks) {
			int dest = __builtin_ctzll(attacks);
			moves.push_back(NEW_MOVE(pos, dest, 0, 0, 0));
			SET0(attacks, dest);
		}
		SET0(pieces, pos);
	}

	ull attacks = get_king_attacks(kingPos) & ~attacked & ~board.pieces[board.turn][ALL];
	while (attacks) {
		int dest = __builtin_ctzll(attacks);
		moves.push_back(NEW_MOVE(kingPos, dest, 0, 0, 0));
		SET0(attacks, dest);
	}

	if (!checks) {
		if (board.turn) {
			if (board.castle[BLACK_SHORT] && !(attacked & 432345564227567616ull) && !(board.allPieces & 432345564227567616ull))
				moves.push_back(NEW_MOVE(kingPos, G8, 1, 0, 0));
			if (board.castle[BLACK_LONG] && !(attacked & 3458764513820540928ull) && !(board.allPieces & 8070450532247928832ull))
				moves.push_back(NEW_MOVE(kingPos, C8, 2, 0, 0));
		}
		else {
			if (board.castle[WHITE_SHORT] && !(attacked & 6ull) && !(board.allPieces & 6ull))
				moves.push_back(NEW_MOVE(kingPos, G1, 1, 0, 0));
			if (board.castle[WHITE_LONG] && !(attacked & 48ull) && !(board.allPieces & 112ull))
				moves.push_back(NEW_MOVE(kingPos, C1, 2, 0, 0));
		}
	}
}

const int pieceValue[6] = { 100, 320, 340, 500, 900, 0 };
bool sort_move_order(std::pair<int, int> o1, std::pair<int, int> o2) {
    return o1.second > o2.second;
}
void movegen::move_gen_with_ordering(const bitboard::Position &board, std::vector<int> &moves) {
    std::vector<int> unorderedMoves;
    movegen::move_gen(board, unorderedMoves);

    std::vector<std::pair<int, int>> score;
    for (const int &move : unorderedMoves) {
        int estimatedScore = 0;

		int source = SOURCE(move);
		int dest = DEST(move);
		int promote = PROMOTE(move);
		int capture = board.mailbox[dest];
		if (capture >= 6)
			capture -= 6;

        if (capture != -1) {
			int moved = board.mailbox[source];
			if (moved >= 6)
				moved -= 6;
			
            int movedPiece = pieceValue[moved];
            int capturedPiece = pieceValue[capture];

            estimatedScore = std::max(capturedPiece - movedPiece, 1) * capturedPiece;
        }
        if (promote)
            estimatedScore += 1000;

        score.push_back({move, estimatedScore});
    }

    std::sort(score.begin(), score.end(), sort_move_order);

    for (const std::pair<int, int> &move : score)
        moves.push_back(move.first);
}