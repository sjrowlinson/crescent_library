#ifndef RANGED_HISTOGRAM_H
#define RANGED_HISTOGRAM_H
#include <algorithm>
#include <array>
#include <map>

namespace crsc {
	namespace detail {
		struct range_less {
			template<class Ty>
			constexpr bool operator()(const std::pair<Ty, Ty>& lhs,
				const std::pair<Ty, Ty>& rhs) const {
				return lhs.first < rhs.first;
			}
		};
	}

	typedef std::map<std::pair<typename Ty, typename Ty>, std::size_t, detail::range_less> ranged_histogram;

	/**
	 * \brief Constructs an histogram in the form of a `std::map` container where the keys
	 *        are ranged bins and the values are `std::size_t` counter for frequency of data.
	 *        Note that all bins are of equal size using this method.
	 * \tparam Ty Type of data stored in iterator range being passed.
	 * \tparam Nbins Number of bins to create in histogram.
	 * \tparam InputIt An iterator satisfying the `InputIterator` concept.
	 * \param first Beginning of range to bin.
	 * \param last End of range to bin.
	 * \return Histogrammed data from the range `[first, last]` in the form of a `std::map`.
	 */
	template<class Ty,
		std::size_t Nbins,
		class InputIt,
		class = std::enable_if_t<std::is_arithmetic<Ty>::value>
	> ranged_histogram make_ranged_histogram(InputIt first, InputIt last) {
		ranged_histogram hist;
		// get extrema of data set
		auto minmax = std::minmax_element(first, last);
		Ty min = std::floor(*minmax.first);
		Ty max = std::ceil(*minmax.second);
		Ty bin_size = (max - min) / Nbins; // equal size bins
		// initialise each bin with zeroed frequency mapped value
		for (std::size_t i = 0U; i < Nbins; ++i)
			hist[std::make_pair(min + i*bin_size, min + (i + 1)*bin_size)] = 0U;
		for (; first < last; ++first) { // iterate through data in range to bin
			for (std::size_t i = 0U; i < Nbins; ++i) { // find correct bin to insert element into
				if (*first > (min + i*bin_size) && *first < (min + (i + 1)*bin_size))
					hist[std::make_pair(min + i*bin_size, min + (i + 1)*bin_size)]++;
			}
		}
		return hist;
	}
	/**
	 * \brief Constructs an histogram in the form of a `std::map` container where the keys
	 *        are ranged bins and the values are `std::size_t` counter for frequency of data.
	 *        A `std::array` of weights is passed to this function to initialise the size
	 *        of each sequential bin via this array of weights.
	 * \tparam Ty Type of data stored in iterator range being passed.
	 * \tparam Nbins Number of bins to create in histogram.
	 * \tparam InputIt An iterator satisfying the `InputIterator` concept.
	 * \param first Beginning of range to bin.
	 * \param last End of range to bin.
	 * \param bin_weights A `std::array` of doubles indicating sequential weights for each bin.
	 * \return Histogrammed data from the range `[first, last]` in the form of a `std::map`.
	 */
	template<class Ty,
		std::size_t Nbins,
		class InputIt,
		class = std::enable_if_t<std::is_arithmetic<Ty>::value>
	> ranged_histogram make_ranged_histogram(InputIt first, InputIt last, std::array<double, Nbins> bin_weights) {
		ranged_histogram hist;
		// get extrema of data set
		auto minmax = std::minmax_element(first, last);
		Ty min = std::floor(*minmax.first);
		Ty max = std::ceil(*minmax.second);
		std::array<Ty, Nbins> bsz_arr;
		// set bin sizes using bin_weights passed to fn
		for (std::size_t i = 0U; i < Nbins; ++i)
			bsz_arr[i] = static_cast<Ty>(bin_weights[i]*(max - min) / Nbins);
		// initialise each bin with zeroed frequency mapped value
		for (std::size_t i = 0U; i < Nbins; ++i)
			hist[std::make_pair(min + i*bsz_arr[i], min + (i + 1)*bsz_arr[i])] = 0U;
		for (; first < last; ++first) { // iterate through data in range to bin
			for (std::size_t i = 0U; i < Nbins; ++i) { // find correct bin to insert element into
				if (*first >(min + i*bsz_arr[i]) && *first < (min + (i + 1)*bsz_arr[i]))
					hist[std::make_pair(min + i*bsz_arr[i], min + (i + 1)*bsz_arr[i])]++;
			}
		}
		return hist;
	}
}

#endif
