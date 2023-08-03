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
	if (QUERY(board.all_pieces, square + 8 * multiplier))
		return 0ull;
	ull moves = (1ull << (square + 8 * multiplier));
	if ((square / 8 == 1 && !board.turn) || (square / 8 == 6 && board.turn))
		moves |= (1ull << (square + 16 * multiplier));

	ull answer = moves & ~board.all_pieces;
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

	bool top_edge = square / 8 > 0;
	bool right_edge = square % 8 < 7;
	bool bottom_edge = square / 8 < 7;
	bool left_edge = square % 8 > 0;

	if (top_edge)
		SET1(attacks, square - 8);
	if (right_edge)
		SET1(attacks, square + 1);
	if (bottom_edge)
		SET1(attacks, square + 8);
	if (left_edge)
		SET1(attacks, square - 1);
	
	if (top_edge && right_edge)
		SET1(attacks, square - 7);
	if (bottom_edge && right_edge)
		SET1(attacks, square + 9);
	if (bottom_edge && left_edge)
		SET1(attacks, square + 7);
	if (top_edge && left_edge)
		SET1(attacks, square - 9);
	
	return attacks;
} 

ull getAttacked(const bitboard::Position &board, bool color) {
	ull pieces_no_king = board.all_pieces ^ board.pieces[color][KING];
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
		attacks |= movegen::get_bishop_attacks(board, pieces_no_king, pos);
		SET0(pieces, pos);
	}

	pieces = board.pieces[color][ROOK];
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		attacks |= movegen::get_rook_attacks(board, pieces_no_king, pos);
		SET0(pieces, pos);
	}
	
	pieces = board.pieces[color][QUEEN];
	while (pieces) {
		int pos = __builtin_ctzll(pieces);
		attacks |= movegen::get_queen_attacks(board, pieces_no_king, pos);
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
	int king_pos = __builtin_ctzll(board.pieces[color][KING]);

	ull checks = get_pawn_attacks(king_pos, color) & board.pieces[!color][PAWN];
	checks |= get_knight_attacks(king_pos) & board.pieces[!color][KNIGHT];
	checks |= get_bishop_attacks(board, board.all_pieces, king_pos) & (board.pieces[!color][BISHOP] | board.pieces[!color][QUEEN]);
	checks |= get_rook_attacks(board, board.all_pieces, king_pos) & (board.pieces[!color][ROOK] | board.pieces[!color][QUEEN]);
	
	return checks;
}
ull getPinned(const bitboard::Position &board, bool color) {
	int pinned_piece, king_pos = __builtin_ctzll(board.pieces[color][KING]);;
	ull blockers, pinned = 0;
	ull own_pieces = board.pieces[color][ALL];
	ull rook_queen = board.pieces[!color][ROOK] | board.pieces[!color][QUEEN];
	ull bishop_queen = board.pieces[!color][BISHOP] | board.pieces[!color][QUEEN];
	
	blockers = maps::rook[king_pos][0] & board.all_pieces;
	if (blockers) {
		pinned_piece = __builtin_ctzll(blockers);
		SET0(blockers, pinned_piece);
		if (blockers) {
			if (QUERY(rook_queen, 63 - __builtin_clzll(~maps::rook[__builtin_ctzll(blockers)][0] & maps::rook[king_pos][0]))) {
				ull potential = 1ull << pinned_piece;
				if (potential & own_pieces)
					pinned |= potential;
			}
		}
	}
	
	blockers = maps::rook[king_pos][1] & board.all_pieces;
	if (blockers) {
		pinned_piece = 63 - __builtin_clzll(blockers);
		SET0(blockers, pinned_piece);
		if (blockers) {
			if (QUERY(rook_queen, __builtin_ctzll(~maps::rook[63 - __builtin_clzll(blockers)][1] & maps::rook[king_pos][1]))) {
				ull potential = 1ull << pinned_piece;
				if (potential & own_pieces)
					pinned |= potential;
			}
		}
	}
	
	blockers = maps::rook[king_pos][2] & board.all_pieces;
	if (blockers) {
		pinned_piece = 63 - __builtin_clzll(blockers);
		SET0(blockers, pinned_piece);
		if (blockers) {
			if (QUERY(rook_queen, __builtin_ctzll(~maps::rook[63 - __builtin_clzll(blockers)][2] & maps::rook[king_pos][2]))) {
				ull potential = 1ull << pinned_piece;
				if (potential & own_pieces)
					pinned |= potential;
			}
		}
	}
	
	blockers = maps::rook[king_pos][3] & board.all_pieces;
	if (blockers) {
		pinned_piece = __builtin_ctzll(blockers);
		SET0(blockers, pinned_piece);
		if (blockers) {
			if (QUERY(rook_queen, 63 - __builtin_clzll(~maps::rook[__builtin_ctzll(blockers)][3] & maps::rook[king_pos][3]))) {
				ull potential = 1ull << pinned_piece;
				if (potential & own_pieces)
					pinned |= potential;
			}
		}
	}
	
	blockers = maps::bishop[king_pos][0] & board.all_pieces;
	if (blockers) {
		pinned_piece = __builtin_ctzll(blockers);
		SET0(blockers, pinned_piece);
		if (blockers) {
			if (QUERY(bishop_queen, 63 - __builtin_clzll(~maps::bishop[__builtin_ctzll(blockers)][0] & maps::bishop[king_pos][0]))) {
				ull potential = 1ull << pinned_piece;
				if (potential & own_pieces)
					pinned |= potential;
			}
		}
	}
	
	blockers = maps::bishop[king_pos][1] & board.all_pieces;
	if (blockers) {
		pinned_piece = 63 - __builtin_clzll(blockers);
		SET0(blockers, pinned_piece);
		if (blockers) {
			if (QUERY(bishop_queen, __builtin_ctzll(~maps::bishop[63 - __builtin_clzll(blockers)][1] & maps::bishop[king_pos][1]))) {
				ull potential = 1ull << pinned_piece;
				if (potential & own_pieces)
					pinned |= potential;
			}
		}
	}
	
	blockers = maps::bishop[king_pos][2] & board.all_pieces;
	if (blockers) {
		pinned_piece = 63 - __builtin_clzll(blockers);
		SET0(blockers, pinned_piece);
		if (blockers) {
			if (QUERY(bishop_queen, __builtin_ctzll(~maps::bishop[63 - __builtin_clzll(blockers)][2] & maps::bishop[king_pos][2]))) {
				ull potential = 1ull << pinned_piece;
				if (potential & own_pieces)
					pinned |= potential;
			}
		}
	}
	
	blockers = maps::bishop[king_pos][3] & board.all_pieces;
	if (blockers) {
		pinned_piece = __builtin_ctzll(blockers);
		SET0(blockers, pinned_piece);
		if (blockers) {
			if (QUERY(bishop_queen, 63 - __builtin_clzll(~maps::bishop[__builtin_ctzll(blockers)][3] & maps::bishop[king_pos][3]))) {
				ull potential = 1ull << pinned_piece;
				if (potential & own_pieces)
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

	int king_pos = __builtin_ctzll(board.pieces[board.turn][KING]);
	if (board.en_passant != -1) {
		int dir = board.turn ? 1 : -1;
		int center = board.en_passant + (dir * 8);

		int pawn = PAWN + (board.turn ? 6 : 0);

		if (board.mailbox[center - 1] == pawn && center % 8 > 0) {
			int move = NEW_MOVE(center - 1, board.en_passant, 0, 0, 1);

			bitboard::Position new_board;
			memcpy(&new_board, &board, sizeof(board));

			move::make_move(new_board, move);
			if (!get_checks(new_board, !new_board.turn))
				moves.push_back(move);
		}
		if (board.mailbox[center + 1] == pawn && center % 8 < 7) {
			int move = NEW_MOVE(center + 1, board.en_passant, 0, 0, 1);

			bitboard::Position new_board;
			memcpy(&new_board, &board, sizeof(board));

			move::make_move(new_board, move);
			if (!get_checks(new_board, !new_board.turn))
				moves.push_back(move);
		}
	}

	if (checks) {
		if (checks & (checks - 1)) { // double check
			ull attacks = get_king_attacks(king_pos) & ~attacked & ~board.pieces[board.turn][ALL];
			while (attacks) {
				int pos = __builtin_ctzll(attacks);
				moves.push_back(NEW_MOVE(king_pos, pos, 0, 0, 0));
				SET0(attacks, pos);
			}
			return;
		}
		
		int checker_pos = __builtin_ctzll(checks);
		int piece = board.mailbox[checker_pos];
		if (piece >= 6)
			piece -= 6;

		if (board.mailbox[checker_pos] == KNIGHT)
			blocks = 1ull << checker_pos;
		else {
			ull filled = (1ull << checker_pos) | (1ull << king_pos);
			filled = maps::fill[__builtin_ctzll(filled)][63 - __builtin_clzll(filled)];

			if (maps::pinned_offsets[piece][king_pos].count(king_pos - checker_pos))
				blocks = maps::pinned_offsets[piece][king_pos].at(king_pos - checker_pos) & filled & ~board.pieces[board.turn][KING];
			else
				blocks = 1ull << checker_pos;
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
		attacks &= maps::pinned_offsets_all[king_pos].at(king_pos - pos);
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
		ull attacks = get_bishop_attacks(board, board.all_pieces, pos) & ~board.pieces[board.turn][ALL] & blocks;
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
		ull attacks = get_bishop_attacks(board, board.all_pieces, pos) & maps::pinned_offsets_all[pos].at(king_pos - pos) & ~board.pieces[board.turn][ALL] & blocks;
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
		ull attacks = get_rook_attacks(board, board.all_pieces, pos) & ~board.pieces[board.turn][ALL] & blocks;
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
		ull attacks = get_rook_attacks(board, board.all_pieces, pos) & maps::pinned_offsets_all[pos].at(king_pos - pos) & ~board.pieces[board.turn][ALL] & blocks;
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
		ull attacks = get_queen_attacks(board, board.all_pieces, pos) & ~board.pieces[board.turn][ALL] & blocks;
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
		ull attacks = get_queen_attacks(board, board.all_pieces, pos) & maps::pinned_offsets_all[pos].at(king_pos - pos) & ~board.pieces[board.turn][ALL] & blocks;
		while (attacks) {
			int dest = __builtin_ctzll(attacks);
			moves.push_back(NEW_MOVE(pos, dest, 0, 0, 0));
			SET0(attacks, dest);
		}
		SET0(pieces, pos);
	}

	ull attacks = get_king_attacks(king_pos) & ~attacked & ~board.pieces[board.turn][ALL];
	while (attacks) {
		int dest = __builtin_ctzll(attacks);
		moves.push_back(NEW_MOVE(king_pos, dest, 0, 0, 0));
		SET0(attacks, dest);
	}

	if (!checks) {
		if (board.turn) {
			if (board.castle[BLACK_SHORT] && !(attacked & 432345564227567616ull) && !(board.all_pieces & 432345564227567616ull))
				moves.push_back(NEW_MOVE(king_pos, G8, 1, 0, 0));
			if (board.castle[BLACK_LONG] && !(attacked & 3458764513820540928ull) && !(board.all_pieces & 8070450532247928832ull))
				moves.push_back(NEW_MOVE(king_pos, C8, 2, 0, 0));
		}
		else {
			if (board.castle[WHITE_SHORT] && !(attacked & 6ull) && !(board.all_pieces & 6ull))
				moves.push_back(NEW_MOVE(king_pos, G1, 1, 0, 0));
			if (board.castle[WHITE_LONG] && !(attacked & 48ull) && !(board.all_pieces & 112ull))
				moves.push_back(NEW_MOVE(king_pos, C1, 2, 0, 0));
		}
	}
}

int piece_value[6] = { 100, 320, 340, 500, 900, 0 };
bool sort_move_order(std::pair<int, int> o1, std::pair<int, int> o2) {
    return o1.second > o2.second;
}
void movegen::move_gen_with_ordering(const bitboard::Position &board, std::vector<int> &moves) {
    std::vector<int> unordered_moves;
    movegen::move_gen(board, unordered_moves);

    std::vector<std::pair<int, int>> score;
    for (const int &move : unordered_moves) {
        int estimated_score = 0;

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
			
            int moved_piece = piece_value[moved];
            int captured_piece = piece_value[capture];

            estimated_score = std::max(captured_piece - moved_piece, 1) * captured_piece;
        }
        if (promote)
            estimated_score += 1000;

        score.push_back({move, estimated_score});
    }

    std::sort(score.begin(), score.end(), sort_move_order);

    for (const std::pair<int, int> &move : score)
        moves.push_back(move.first);
}