#ifndef ALGORITHM_UTILITIES_H
#define ALGORITHM_UTILITIES_H
#include <algorithm>
#include <random>
#include <set>
#include <vector>

namespace crsc {
	/**
	 * \brief Tag-sorts a `std::vector` returning the sorted vector of indices.
	 *
	 * \tparam The type of the elements stored in `vec`.
	 * \param vec `std::vector` of data.
	 * \return `std::vector` of indices ordered by sorting of `_vec`.
	 */
	template<typename Ty>
	std::vector<std::size_t> tag_sort(const std::vector<Ty>& vec) {
		// allocate std::vector for tag indices with capacity of _vec
		std::vector<std::size_t> tag_vec(vec.size());
		// fill tag_vec with ascending positive integers representing indices
		std::iota(std::begin(tag_vec), std::end(tag_vec), 0);
		// sort tag_vec using a lambda based on ordering of _vec
		std::sort(std::begin(tag_vec), std::end(tag_vec), [&vec](const auto& lhs, const auto& rhs) {
			return vec[lhs] < vec[rhs];
		});
		return tag_vec;
	}
	template<class Ty>
	std::vector<std::pair<Ty, Ty>> zip(const std::vector<Ty>& vec1, const std::vector<Ty>& vec2) {
		std::vector<std::pair<Ty, Ty>> zipped;
		zipped.reserve(std::max(vec1.size(), vec2.size()));
		for (auto it1 = vec1.begin(), it2 = vec2.begin(); it1 < vec1.end() || it2 < vec2.end(); ++it1, ++it2)
			zipped.push_back(std::make_pair(*it1, *it2));
		return zipped;
	}
	template<class Ty>
	std::pair<std::vector<Ty>, std::vector<Ty>> unzip(const std::vector<std::pair<Ty, Ty>>& zipped_vec) {
		std::vector<Ty> vec1, vec2;
		vec1.reserve(zipped_vec.size());
		vec2.reserve(zipped_vec.size());
		for (const auto& p : zipped_vec) {
			vec1.push_back(p.first); 
			vec2.push_back(p.second);
		}
		return{ vec1, vec2 };
	}
	/**
	 * \brief Returns a random element from the range `[first, last)`.
	 * \param first Beginning of the range.
	 * \param last End of the range.
	 * \return Iterator to random element within the range `[first, last)`.
	 */
	template<class InputIt,
        class Engine = std::mt19937
    > InputIt random_element(InputIt first, InputIt last, Engine& eng = std::mt19937{std::random_device{}()}) {
		std::size_t range_size = std::distance(first, last);
		std::uniform_int_distribution<std::size_t> dist(0U, range_size);
		std::advance(first, dist(eng));
		return first;
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
	template<class InputIt, class Ty>
	std::set<InputIt> find_all(InputIt first, InputIt last, const Ty& value) {
		std::set<InputIt> found_set;
		for (; first != last; ++first) {
			if (*first == value) found_set.insert(first);
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
		for (; first != last; ++first) {
			if (p(*first)) found_set.insert(first);
		}
		return found_set;
	}
	/**
	 * \brief Searches for elements in the range `[first, last)` for which the predicate `q` returns
	 *        `false` and returns a `std::set<InputIt>` containing all occcurrences of these elements.
	 *
	 * \param first Beginning of range to examine.
	 * \param last End of range to examine.
	 * \param q Unary predicate which returns `false` for the required elements.
	 * \return `std::set<InputIt>` containing iterators to all occurrencs for which `q` yields `false`.
	 */
	template<class InputIt, class UnaryPredicate>
	std::set<InputIt> find_all_if_not(InputIt first, InputIt last, UnaryPredicate q) {
		std::set<InputIt> found_set;
		for (; first != last; ++first) {
			if (!q(*first)) found_set.insert(first);
		}
		return found_set;
	}
	/**
	 * \brief Fills the range `[first, last)` with sequentially decreasing values, starting with `value` and
	 *        repetitively evaluating `--value`.
	 *
	 * \param first Beginning of range to fill.
	 * \param last End of range to fill.
	 * \param value Initial value to store, `--value` must be well-formed.
	 */
	template<class ForwardIterator, class Ty>
	void iota_opp(ForwardIterator first, ForwardIterator last, Ty value) {
		while (first != last) {
			*first++ = value;
			--value;
		}
	}
}

#endif