#include <iostream>

#include "bitboard.h"
#include "maps.h"

ull maps::knight[64];
ull maps::rook[64][4]; // 0 = north, 1 = east, 2 = south, 3 = west, 4 = all
ull maps::bishop[64][4]; // 0 = northeast, 1 = southeast, 2 = southwest, 3 = northwest, 4 = all
ull maps::fill[64][64];
std::unordered_map<int, ull> maps::pinnedOffsets[6][64];
std::unordered_map<int, ull> maps::pinnedOffsetsAll[64];
	
void maps::init() {
	for (int i = 0; i < 64; i++) {
		for (int y = i / 8; y < 8; y++)
			SET1(maps::rook[i][0], y * 8 + (i % 8));
		for (int x = i % 8; x >= 0; x--)
			SET1(maps::rook[i][1], x + (i / 8) * 8);
		for (int y = i / 8; y >= 0; y--)
			SET1(maps::rook[i][2], y * 8 + (i % 8));
		for (int x = i % 8; x < 8; x++)
			SET1(maps::rook[i][3], x + (i / 8) * 8);

		for (int x = i % 8, y = i / 8; x >= 0 && y < 8; x--, y++)
			SET1(maps::bishop[i][0], y * 8 + x);
		for (int x = i % 8, y = i / 8; x >= 0 && y >= 0; x--, y--)
			SET1(maps::bishop[i][1], y * 8 + x);
		for (int x = i % 8, y = i / 8; x < 8 && y >= 0; x++, y--)
			SET1(maps::bishop[i][2], y * 8 + x);
		for (int x = i % 8, y = i / 8; x < 8 && y < 8; x++, y++)
			SET1(maps::bishop[i][3], y * 8 + x);

		for (int j = 0; j < 4; j++) {
			SET0(maps::rook[i][j], i);
			SET0(maps::bishop[i][j], i);
		}

		int x = i / 8, y = i % 8;
		if (x + 2 < 8 && y + 1 < 8)
			SET1(maps::knight[i], (x + 2) * 8 + y + 1);
		if (x + 1 < 8 && y + 2 < 8)
			SET1(maps::knight[i], (x + 1) * 8 + y + 2);
		if (x - 2 >= 0 && y + 1 < 8)
			SET1(maps::knight[i], (x - 2) * 8 + y + 1);
		if (x + 1 < 8 && y - 2 >= 0)
			SET1(maps::knight[i], (x + 1) * 8 + y - 2);
		if (x - 1 >= 0 && y + 2 < 8)
			SET1(maps::knight[i], (x - 1) * 8 + y + 2);
		if (x + 2 < 8 && y - 1 >= 0)
			SET1(maps::knight[i], (x + 2) * 8 + y - 1);
		if (x - 1 >= 0 && y - 2 >= 0)
			SET1(maps::knight[i], (x - 1) * 8 + y - 2);
		if (x - 2 >= 0 && y - 1 >= 0)
			SET1(maps::knight[i], (x - 2) * 8 + y - 1);

		for (int j = 1; j <= 7; j++) {
			maps::pinnedOffsets[BISHOP][i].insert({ j * -7, maps::bishop[i][0] });
			maps::pinnedOffsets[BISHOP][i].insert({ j * 7, maps::bishop[i][2] });
			maps::pinnedOffsets[BISHOP][i].insert({ j * 9, maps::bishop[i][1] });
			maps::pinnedOffsets[BISHOP][i].insert({ j * -9, maps::bishop[i][3] });
			maps::pinnedOffsets[QUEEN][i].insert({ j * -7, maps::bishop[i][0] });
			maps::pinnedOffsets[QUEEN][i].insert({ j * 7, maps::bishop[i][2] });
			maps::pinnedOffsets[QUEEN][i].insert({ j * 9, maps::bishop[i][1] });
			maps::pinnedOffsets[QUEEN][i].insert({ j * -9, maps::bishop[i][3] });
			maps::pinnedOffsets[ROOK][i].insert({ j * -8, maps::rook[i][0] });
			maps::pinnedOffsets[ROOK][i].insert({ j * 8, maps::rook[i][2] });
			maps::pinnedOffsets[ROOK][i].insert({ j * 1, maps::rook[i][1] });
			maps::pinnedOffsets[ROOK][i].insert({ j * -1, maps::rook[i][3] });
			maps::pinnedOffsets[QUEEN][i].insert({ j * -8, maps::rook[i][0] });
			maps::pinnedOffsets[QUEEN][i].insert({ j * 8, maps::rook[i][2] });
			maps::pinnedOffsets[QUEEN][i].insert({ j * 1, maps::rook[i][1] });
			maps::pinnedOffsets[QUEEN][i].insert({ j * -1, maps::rook[i][3] });

			maps::pinnedOffsetsAll[i].insert({ j * -8, maps::rook[i][0] | maps::rook[i][2] });
			maps::pinnedOffsetsAll[i].insert({ j * 8, maps::rook[i][0] | maps::rook[i][2] });
			maps::pinnedOffsetsAll[i].insert({ j * 1, maps::rook[i][1] | maps::rook[i][3] });
			maps::pinnedOffsetsAll[i].insert({ j * -1, maps::rook[i][1] | maps::rook[i][3] });
			maps::pinnedOffsetsAll[i].insert({ j * -7, maps::bishop[i][0] | maps::bishop[i][2] });
			maps::pinnedOffsetsAll[i].insert({ j * 7, maps::bishop[i][0] | maps::bishop[i][2] });
			maps::pinnedOffsetsAll[i].insert({ j * 9, maps::bishop[i][1] | maps::bishop[i][3] });
			maps::pinnedOffsetsAll[i].insert({ j * -9, maps::bishop[i][1] | maps::bishop[i][3] });
		}

		for (int j = 0; j < 64; j++) {
			for (int k = i; k <= j; k++)
				SET1(maps::fill[i][j], k);
		}
	}
}