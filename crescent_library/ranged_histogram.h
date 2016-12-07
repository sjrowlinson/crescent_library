#ifndef RANGED_HISTOGRAM_H
#define RANGED_HISTOGRAM_H
#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <type_traits>

namespace hist {
	template<class RTy,
		std::size_t NBins,
		class = std::enable_if_t<std::is_arithmetic<RTy>::value>
	> class ranged_histogram {
		struct range_less { // less comparator for a range std::pair
			template<class Ty>
			constexpr bool operator()(const std::pair<Ty, Ty>& lhs,
				const std::pair<Ty, Ty>& rhs) const {
				return lhs.first < rhs.first;
			}
		};
		typedef std::map<std::pair<RTy, RTy>, std::size_t, range_less> rhist_t;
	public:
		typedef RTy range_type; // type of range limits
		typedef std::pair<RTy, RTy> key_type; // key of std::map
											  // default constructor
		ranged_histogram() : rh(), bin_size_(range_type()) {}
		template<class InputIt>
		ranged_histogram(InputIt first, InputIt last)
			: rh(), bin_size_(range_type()) { // construct from data range
											  // get extrema of data set
			auto minmax = std::minmax_element(first, last);
			range_type min = std::floor(*minmax.first);
			range_type max = std::ceil(*minmax.second);
			bin_size_ = (max - min) / NBins; // equal size bins
											 // compute bin size reciprocal to avoid repeated divisions
			range_type bs_recip = 1.0 / bin_size_;
			// zero initialise frequencies of each bin
			for (std::size_t i = 0U; i < NBins; ++i)
				rh[std::make_pair(min + i*bin_size_, min + (i + 1)*bin_size_)] = 0U;
			for (; first < last; ++first) { // iterate through data in range to bin
				std::size_t bin = (*first - min)*bs_recip; // find correct bin
				rh[std::make_pair(min + bin*bin_size_, min + (bin + 1)*bin_size_)]++;
			}
		}
		// number of bins in histogram
		constexpr std::size_t bins() const noexcept { return NBins; }
		// size of each bin
		range_type bin_size() const noexcept { return bin_size_; }
		// accessor for frequency of a a given bin
		std::size_t& operator[](const key_type& key) { return rh[key]; }
		// make a std::map of the mid-points of the bin ranges
		std::map<range_type, std::size_t> binned_midranges() const {
			std::map<range_type, std::size_t> mid_hist;
			for (const auto& p : rh)
				mid_hist[(p.first.first + p.first.second) / 2.0] = p.second;
			return mid_hist;
		}
	private:
		rhist_t rh;
		range_type bin_size_;
	};

	template<class RTy,
		std::size_t XNBins,
		std::size_t YNBins,
		class = std::enable_if_t<std::is_arithmetic<RTy>::value>
	> class ranged_histogram_2d {
		typedef std::map<std::pair<
			std::pair<RTy, RTy>,
			std::pair<RTy, RTy>>, std::size_t> rhist_t;
	public:
		typedef RTy range_type; // type of range limits
		typedef std::pair<std::pair<RTy, RTy>, std::pair<RTy, RTy>> key_type; // key of std::map
																			  // default constructor
		ranged_histogram_2d() : rh(), xbin_size(range_type()), ybin_size(range_type()) {}
		template<class InputIt>
		ranged_histogram_2d(InputIt first_x, InputIt last_x,
			InputIt first_y, InputIt last_y) : rh(),
			xbin_size(range_type()), ybin_size(range_type()) { // construct from data ranges
															   // get extrema of each data set
			auto minmax_x = std::minmax_element(first_x, last_x);
			auto minmax_y = std::minmax_element(first_y, last_y);
			range_type min_x = std::floor(*minmax_x.first);
			range_type max_x = std::ceil(*minmax_x.second);
			range_type min_y = std::floor(*minmax_y.first);
			range_type max_y = std::ceil(*minmax_y.second);
			// equal size bins
			xbin_size = (max_x - min_x) / XNBins;
			ybin_size = (max_y - min_y) / YNBins;
			// compute reciprocals of bin sizes to avoid
			// repeated divisions in binning loop
			range_type bs_x_recip = 1.0 / xbin_size;
			range_type bs_y_recip = 1.0 / ybin_size;
			// zero initialise frequencies of each bin
			for (std::size_t i = 0U; i < XNBins; ++i) {
				for (std::size_t j = 0U; j < YNBins; ++j)
					rh[std::make_pair(
						std::make_pair(min_x + i*xbin_size, min_x + (i + 1)*xbin_size),
						std::make_pair(min_y + j*ybin_size, min_y + (j + 1)*ybin_size)
					)] = 0U;
			}
			// iterate through data in ranges to bin
			for (; first_x < last_x || first_y < last_y; ++first_x, ++first_y) {
				std::size_t bin_x = (*first_x - min_x)*bs_x_recip; // find correct bin for x data
				std::size_t bin_y = (*first_y - min_y)*bs_y_recip; // find correct bin for y data
				rh[std::make_pair(
					std::make_pair(min_x + bin_x*xbin_size, min_x + (bin_x + 1)*xbin_size),
					std::make_pair(min_y + bin_y*ybin_size, min_y + (bin_y + 1)*ybin_size)
				)]++;
			}
		}
		// number of bins in x dimension
		constexpr std::size_t xbins() const noexcept { return XNBins; }
		// number of bins in y dimension
		constexpr std::size_t ybins() const noexcept { return YNBins; }
		// size of x dimension bin
		range_type bin_size_x() const noexcept { return xbin_size; }
		// size of y dimension bin
		range_type bin_size_y() const noexcept { return ybin_size; }
		// accessor for frequency of a given bin
		std::size_t& operator[](const key_type& key) { return rh[key]; }
		// make a std::map of the mid-points of the bin ranges
		std::map<std::pair<range_type, range_type>, std::size_t>
			binned_midranges() const {
			std::map<std::pair<range_type, range_type>, std::size_t> mid_hist;
			for (const auto& p : rh)
				mid_hist[std::make_pair(
				(p.first.first.first + p.first.first.second) / 2.0,
					(p.first.second.first + p.first.second.second) / 2.0
				)] = p.second;
			return mid_hist;
		}
		// marginalise out the y-dimension of the histogram, making a ranged_histogram
		ranged_histogram<range_type, XNBins> marginalise_y() const {
			ranged_histogram<range_type, XNBins> marginalised;
			std::size_t i = 0U; std::size_t acc = 0U;
			for (const auto& p : rh) {
				acc += p.second; // accumulate frequencies for each x bin 
				if (!(i % XNBins)) { // check for multiple of XNBins
					marginalised[p.first.first] =
						static_cast<std::size_t>(xbin_size*acc);
					acc = 0U;
				}
				++i;
			}
			return marginalised;
		}
	private:
		rhist_t rh;
		range_type xbin_size;
		range_type ybin_size;
	};
	template<class RTy,
		std::size_t NBins,
		class InputIt
	> ranged_histogram<RTy, NBins> make_ranged_histogram(InputIt first, InputIt last) {
		return ranged_histogram<RTy, NBins>(first, last);
	}
	template<class RTy,
		std::size_t XNBins,
		std::size_t YNBins,
		class InputIt
	> ranged_histogram_2d<RTy, XNBins, YNBins> make_ranged_histogram_2d(InputIt first_x,
		InputIt last_x, InputIt first_y, InputIt last_y) {
		return ranged_histogram_2d<RTy, XNBins, YNBins>(first_x, last_x, first_y, last_y);
	}
}

#endif // !RANGED_HISTOGRAM_H