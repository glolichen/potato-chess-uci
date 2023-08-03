#ifndef MAPS_H
#define MAPS_H

#include <unordered_map>

using ull = unsigned long long;
namespace maps {	
	extern ull knight[64];
	extern ull rook[64][4];
	extern ull bishop[64][4];
	extern ull fill[64][64];
	extern std::unordered_map<int, ull> pinned_offsets[6][64];
	extern std::unordered_map<int, ull> pinned_offsets_all[64];
	
	void init();
}

#endif