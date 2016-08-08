#ifndef ALGORITHM_UTILITIES_H
#define ALGORITHM_UTILITIES_H
#include <algorithm>
#include <numeric>
#include <iostream>
#include <set>
#include <vector>

namespace crsc {
	/**
	 * \brief Tag-sorts a `std::vector` returning the sorted vector of indices.
	 *
	 * \param _vec `std::vector` of data.
	 * \return `std::vector` of indices ordered by sorting of `_vec`.
	 */
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
	/**
	 * \brief Searches for elements with value `value` in the range `[first, last)` and returns
	 *        a `std::set<InputIt>` containing all occurrences of this element.
	 *
	 * \param first Beginning of the range to examine.
	 * \param last End of range to examine.
	 * \param value Value to compare the elements to.
	 * \return `std::set<InputIt>` containing iterators to all occurrences of `value` in the range.
	 */
	template<class InputIt, class _Ty>
	std::set<InputIt> find_all(InputIt first, InputIt last, const _Ty& value) {
		std::set<InputIt> found_set;
		for (InputIt it = first; it != last; ++it) {
			if (*it == value) found_set.insert(it);
		}
		return found_set;
	}
	/**
	 * \brief Searches for elements in the range `[first, last)` for which the predicate `p` returns
	 *        `true` and returns a `std::set<InputIt>` containing all occurrences of these elements.
	 *
	 * \param first Beginning of range to examine.
	 * \param last End of range to examine.
	 * \param p Unary predicate which returns `true` for the required elements.
	 * \return `std::set<InputIt>` containing iterators to all occurrencs for which `p` yields `true`.
	 */
	template<class InputIt, class UnaryPredicate>
	std::set<InputIt> find_all_if(InputIt first, InputIt last, UnaryPredicate p) {
		std::set<InputIt> found_set;
		for (InputIt it = first; it != last; ++it) {
			if (p(*it)) found_set.insert(it);
		}
		return found_set;
	}
	/**
	 * \brief Searches for elements in the range `[first, last)` for which the predicate `q` returns
	 *        `false` and returns a `std::set<InputIt>` containing all occcurrences of these elements.
	 *
	 * \param first Beginning of range to examine.
	 * \param last End of range to examine.
	 * \param q Unary predicate which return `false` for the required elements.
	 * \return `std::set<InputIt>` containing iterators to all occurrencs for which `q` yields `false`.
	 */
	template<class InputIt, class UnaryPredicate>
	std::set<InputIt> find_all_if_not(InputIt first, InputIt last, UnaryPredicate q) {
		std::set<InputIt> found_set;
		for (InputIt it = first; it != last; ++it) {
			if (!q(*it)) found_set.insert(it);
		}
		return found_set;
	}
}

#endif