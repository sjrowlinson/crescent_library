#ifndef ALGORITHM_UTILITIES_H
#define ALGORITHM_UTILITIES_H
#include <algorithm>
#include <numeric>
#include <iostream>
#include <vector>

namespace crsc {

	// sorts a generic std::vector by index tags
	template<typename _Ty> std::vector<std::size_t> tag_sort(const std::vector<_Ty>& _vec) {
		// allocate std::vector for tag indices with capacity of _vec
		std::vector<std::size_t> tag_vec(_vec.size());
		// fill tag_vec with ascending positive integers representing indices
		std::iota(std::begin(tag_vec), std::end(tag_vec), 0);
		// sort tag_vec using a lambda based on ordering of _vec
		std::sort(std::begin(tag_vec), std::end(tag_vec), [&_vec](const auto& _lhs, const auto& _rhs) {
			return _vec[_lhs] < _vec[rhs];
		});
		return tag_vec;
	}

}

#endif