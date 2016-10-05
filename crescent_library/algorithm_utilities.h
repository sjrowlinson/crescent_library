#ifndef ALGORITHM_UTILITIES_H
#define ALGORITHM_UTILITIES_H
#include "randomness.h"
#include <algorithm>
#include <map>
#include <numeric>
#include <set>
#include <type_traits>
#include <unordered_map>
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
	/**
	 * \brief Tag-sorts elements in the range `[first, last)` returning a `std::vector<std::size_t>`
	 *        of the sorted indices.
	 *
	 * \param first Beginning of the range to tag-sort.
	 * \param last End of the range to tag-sort.
	 * \return `std::vector<std::size_t>` of indices ordered by sorting of range.
	 */
	template<class InputIt>
	std::vector<std::size_t> tag_sort(InputIt first, InputIt last) {
		std::vector<std::size_t> tag_vec(std::distance(first, last));
		std::iota(std::begin(tag_vec), std::end(tag_vec), 0);
		std::sort(std::begin(tag_vec), std::end(tag_vec), [&first, &last](const auto& lhs, const auto& rhs) {
			return *first < *(++first);
		});
		return tag_vec;
	}
	/**
	 * \brief Returns a random element from the range `[first, last)`.
	 * \param first Beginning of the range.
	 * \param last End of the range.
	 * \return Iterator to random element within the range `[first, last)`.
	 */
	template<class InputIt>
	InputIt random_element(InputIt first, InputIt last) {
		std::size_t range_size = std::distance(first, last);
		crsc::random_number_generator<std::size_t> rng(std::uniform_int_distribution<std::size_t>(0U, range_size));
		std::advance(first, rng(eng));
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
	 * \brief Searches for elements with value `value_pair.first` in the range `[first, last)` and
	 *        replaces these elements with the value `value_pair.second`, returning the number of
	 *        replace operations performed.
	 * \param first Beginning of range to examine.
	 * \param last End of range to examine.
	 * \param value_pair Pair where `first` corresponds to value to find and `second` is value to
	 *        replace with.
	 */
	template<class InputIt, class Ty>
	std::size_t find_and_replace_all(InputIt first, InputIt last, const std::pair<Ty, Ty>& value_pair) {
		std::size_t nreplaced = 0U;
		for (; first != last; ++first) {
			if (*first == value_pair.first) { *first = value_pair.second; ++nreplaced; }
		}
		return nreplaced;
	}
	/**
	 * \brief Searches for elements in the range `[first, last)` for which the predicate `p` returns
	 *        `true` and replaces these elements with a copy of `value`. The number of replace
	 *        operations performed is returned.
	 *
	 * \param first Beginning of range to examine.
	 * \param last End of range to examine.
	 * \param value Value to replace elements with.
	 * \param p Unary predicate which returns `true` for the required elements.
	 */
	template<class InputIt, class Ty, class UnaryPredicate>
	std::size_t find_and_replace_all_if(InputIt first, InputIt last, const Ty& value, UnaryPredicate p) {
		std::size_t nreplaced = 0U;
		for (; first != last; ++first) {
			if (p(*first)) { *first = value; ++nreplaced; }
		}
		return nreplaced;
	}
	/**
	 * \brief Searches for elements in the range `[first, last)` for which the predicate `q` returns
	 *        `false` and replaces these elements with a copy of `value`. The number of replace
	 *        operations performed is returned.
	 *
	 * \param first Beginning of range to examine.
	 * \param last End of range to examine.
	 * \param value Value to replace elements with.
	 * \param q Unary predicate which returns `false` for the required elements.
	 */
	template<class InputIt, class Ty, class UnaryPredicate>
	std::size_t find_and_replace_all_if_not(InputIt first, InputIt last, const Ty& value, UnaryPredicate q) {
		std::size_t nreplaced = 0U;
		for (; first != last; ++first) {
			if (!q(*first)) { *first = value; ++nreplaced; }
		}
		return nreplaced;
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
	/**
	 * \brief Makes an `std::map` acting as a "singular-key" histogram produced from the data in the
	 *        range `[first, last)`, where the `key_type` is the type of the data in this range
	 *        and the `value_type` is an integral type. Each element within the range is given
	 *        its own bin in the histogram.
	 * \tparam InputIt Type satisfying `Input Iterator` concept.
	 * \tparam KeyType The type of map key, undefined behaviour if `KeyType` not equal to type 
	 *                 of elements in the range `[first, last)`.
	 * \tparam ValueType The type of map value, must satisfy `std::is_integral<ValueType>`.
	 * \tparam Compare The comparator used for key-comparisons in the map.
	 * \tparam Allocator An allocator that is used to acquire memory to store the elements. The type must meet the requirements of
	 *         `Allocator` (see C++ Standard). Behaviour is undefined if `Allocator::value_type != std::pair<const KeyType, ValueType>`.
	 * \param InputIt first Beginning of data range.
	 * \param InputIt last End of data range.
	 * \param alloc Allocator to use for all memory allocations of this container.
	 * \complexity Linear in `distance` between `first` and `last`, multiplied by logarithmic in `distance` between `first`
	 *             and `last`.
	 */
	template<class InputIt, 
		class KeyType, 
		class ValueType = std::size_t,
		class Compare = std::less<KeyType>,
		class Allocator = std::allocator<std::pair<const KeyType, ValueType>>,
		class = std::enable_if_t<std::is_integral<ValueType>::value>
	> std::map<KeyType, ValueType, Compare, Allocator> construct_ordered_histogram(InputIt first, InputIt last, const Allocator& alloc = Allocator()) {
		using key_type = KeyType; using value_type = ValueType; 
		using compare_type = Compare; using allocator_type = Allocator;
		std::map<key_type, value_type, compare_type, allocator_type> histogram;
		for (; first != last; ++first) ++histogram[*first];	// increment counter for each occurrence of an element
		return histogram;
	}
	/**
	 * \brief Makes an `std::unordered_map` acting as a "singular-key" histogram produced from the data in the
	 *        range `[first, last)`, where the `key_type` is the type of the data in this range
	 *        and the `value_type` is an integral type. Each element within the range is given
	 *        its own bin in the histogram.
	 * \tparam InputIt Type satisfying `Input Iterator` concept.
	 * \tparam KeyType The type of map key, undefined behaviour if `KeyType` not equal to type 
	 *                 of elements in the range `[first, last)`.
	 * \tparam ValueType The type of map value, must satisfy `std::is_integral<ValueType>`.
	 * \tparam Hash The type of hasher used to determine hash function for `KeyType`.
	 * \tparam KeyEqual The type of equality comparator for `KeyType`.
	 * \tparam Allocator An allocator that is used to acquire memory to store the elements. The type must meet the requirements of
	 *         `Allocator` (see C++ Standard). Behaviour is undefined if `Allocator::value_type != std::pair<const KeyType, ValueType>`.
	 * \param InputIt first Beginning of data range.
	 * \param InputIt last End of data range.
	 * \param alloc Allocator to use for all memory allocations of this container.
	 * \complexity - Average Case: linear in `distance` between `first` and `last`.
	 *             - Worst Case: quadratic in `distance` between `first` and `last`.
	 */
	template<class InputIt,
		class KeyType,
		class ValueType = std::size_t,
		class Hash = std::hash<KeyType>,
		class KeyEqual = std::equal_to<KeyType>,
		class Allocator = std::allocator<std::pair<const KeyType, ValueType>>,
		class = std::enable_if_t<std::is_integral<ValueType>::value>
	> std::unordered_map<KeyType, ValueType, Hash, KeyEqual, Allocator> construct_unordered_histogram(InputIt first, InputIt last, const Allocator& alloc = Allocator()) {
		using key_type = KeyType; using value_type = ValueType; using hash_type = Hash;
		using key_equal_type = KeyEqual; using allocator_type = Allocator;
		std::unordered_map<key_type, value_type, hash_type, key_equal_type, allocator_type> histogram;
		for (; first != last; ++first) ++histogram[*first]; // increment counter for each occurrence of an element
		return histogram;
	}
}

#endif