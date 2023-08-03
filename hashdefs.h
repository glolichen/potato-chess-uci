#ifndef HASHDEFS_H
#define HASHDEFS_H

#include <tuple>
#include <unordered_map>

namespace hashdefs {
	using ull = unsigned long long;

	// std::tuple as key in unordered map: https://stackoverflow.com/a/20835070
	using ZobristTuple = std::tuple<ull, ull, ull>;
	struct ZobristTupleHash {
		std::size_t operator()(const ZobristTuple& k) const {
			return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k);
		}
	};
}

#endif