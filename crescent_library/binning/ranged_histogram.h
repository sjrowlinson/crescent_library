#ifndef RANGED_HISTOGRAM_H
#define RANGED_HISTOGRAM_H
#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <type_traits>

namespace crsc {
    namespace hist {
        template<class RTy,
            class = std::enable_if_t<std::is_arithmetic<RTy>::value>
        > class ranged_histogram {
            // underlying container for ranged_histogram
            typedef std::map<std::pair<RTy, RTy>, std::size_t> rhist_t;
        public:
            // PUBLIC TYPEDEFS
            typedef RTy range_type;
            typedef std::pair<RTy, RTy> bin_type;
            typedef std::size_t frequency_type;
            typedef typename rhist_t::const_iterator const_iterator;
            typedef typename rhist_t::const_reverse_iterator const_reverse_iterator;
            // CONSTRUCTION / DESTRUCTION
            /**
             * \brief Construct empty `ranged_histogram` with no bins.
             */
            ranged_histogram() : rh(), nbins(0U), bin_size(range_type()) {}
            /**
             * brief Construct `ranged_histogram` with `_nbins` bins of equal width
             *       from a range of data `[first, last)`.
             * \param first Beginning of data range to bin.
             * \param last End of data range to bin.
             * \param _nbins Number of equal-width bins to construct in histogram.
             */
            template<class InputIt>
            ranged_histogram(InputIt first, InputIt last, std::size_t _nbins) 
                : rh(), bin_size(range_type()) {
                bin_data_(first, last, _nbins);
            }
            ranged_histogram(rhist_t&& range_map) : rh(std::move(range_map)),
                nbins(range_map.size()) {
                bin_size = (*range_map.begin()).second - (*range_map.begin()).first;    
            }           
            // BIN PROPERTIES
            /**
             * \brief Returns the number of bins in the histogram.
             * \return The number of bins.
             */
            std::size_t bins() const noexcept { return nbins; }
            range_type bin_width() const noexcept { return bin_size; }
            // DATA BINNING
            /**
             * \brief Bins the data in the range `[first, last)` using equal-width bins.
             * \param first Beginning of data range to bin.
             * \param last End of data range to bin.
             * \param _nbins Number of equal-width bins to construct in histogram.
             */
            template<class InputIt>
            void bin_data(InputIt first, InputIt last, std::size_t _nbins) {
                bin_data_(first, last, _nbins);
            }
            // FREQUENCY ACCESS
            /**
             * \brief Read-only access of freqeuncy for a given bin.
             * \return The frequency of the given bin.
             */
            frequency_type operator[](const bin_type& bin) const noexcept { return rh[bin]; }
            /**
             * \brief Read-only access of freqeuncy for a given bin.
             * \return The frequency of the given bin.
             */
            frequency_type operator[](bin_type&& bin) const noexcept { return rh[std::move(bin)]; }
            // ITERATORS
            const_iterator begin() const noexcept { return rh.begin(); }
            const_iterator cbegin() const noexcept { return rh.cbegin(); }
            const_iterator end() const noexcept { return rh.end(); }
            const_iterator cend() const noexcept { return rh.cend(); }
            const_reverse_iterator rbegin() const noexcept { return rh.rbegin(); }
            const_reverse_iterator crbegin() const noexcept { return rh.crbegin(); }
            const_reverse_iterator rend() const noexcept { return rh.rend(); }
            const_reverse_iterator crend() const noexcept { return rh.crend(); }
        private:
            template<class InputIt>
            void bin_data_(InputIt first, InputIt last, std::size_t _nbins) {
                nbins = _nbins;
                auto minmax = *std::minmax_element(first, last);
                // get min and max of data range
                range_type min = std::floor(minmax.first);
                range_type max = std::floor(minmax.second);
                bin_size = (max - min) / nbins;
                // store reciprocal of bin size for computation speed
                range_type bs_recip = static_cast<range_type>(1.0 / bin_size);
                // zero-initialise frequencies of each bin
                for (std::size_t i = 0U; i < nbins; ++i)
                    rh[std::make_pair(min + i*bin_size, min + (i + 1)*bin_size)] = 0U;
                for (; first < last; ++first) { // bin data 
                    std::size_t bin = (*first - min)*bs_recip; // find correct bin
                    rh[std::make_pair(min + bin*bin_size, min + (bin + 1)*bin_size)]++;
                }
            }
            rhist_t rh;
            std::size_t nbins;
            range_type bin_size;
        };

        template<class RTy,
            class = std::enable_if_t<std::is_arithmetic<RTy>::value>
        > class ranged_histogram_2d {
            typedef std::map<std::pair<
                std::pair<RTy, RTy>,
                std::pair<RTy, RTy>>, std::size_t> rhist_t;
        public:
            // PUBLIC TYPEDEFS
            typedef RTy range_type;
            typedef std::pair<std::pair<RTy, RTy>, std::pair<RTy, RTy>> bin_type;
            typedef std::size_t frequency_type;
            typedef typename rhist_t::const_iterator const_iterator;
            typedef typename rhist_t::const_reverse_iterator const_reverse_iterator;
            // CONSTRUCTION / DESTRUCTION
            ranged_histogram_2d() : rh(), nbinsx(0U), nbinsy(0U), 
                xbin_size(range_type()), ybin_size(range_type()) {}
            template<class InputIt>
            ranged_histogram_2d(InputIt first_x, InputIt last_x, InputIt first_y, InputIt last_y,
                std::size_t xbins, std::size_t ybins) : rh() {
                bin_data_(first_x, last_x, first_y, last_y, xbins, ybins);
            }
            // BIN PROPERTIES
            std::size_t xbins() const noexcept { return nbinsx; }
            std::size_t ybins() const noexcept { return nbinsy; }
            range_type xbin_width() const noexcept { return xbin_size; }
            range_type ybin_width() const noexcept { return ybin_size; }
            // DATA BINNING
            template<class InputIt>
            void bin_data(InputIt first_x, InputIt last_x, InputIt first_y, InputIt last_y, 
                std::size_t xbins, std::size_t ybins) {
                bin_data_(first_x, last_x, first_y, last_y, xbins, ybins);
            }
            // FREQUENCY ACCESS
            frequency_type operator[](const bin_type& bin) { return rh[bin]; }
            frequency_type operator[](bin_type&& bin) { return rh[std::move(bin)]; }
            // ITERATORS
            const_iterator begin() const noexcept { return rh.begin(); }
            const_iterator cbegin() const noexcept { return rh.cbegin(); }
            const_iterator end() const noexcept { return rh.end(); }
            const_iterator cend() const noexcept { return rh.cend(); }
            const_reverse_iterator rbegin() const noexcept { return rh.rbegin(); }
            const_reverse_iterator crbegin() const noexcept { return rh.crbegin(); }
            const_reverse_iterator rend() const noexcept { return rh.rend(); }
            const_reverse_iterator crend() const noexcept { return rh.crend(); }
        private:
            template<class InputIt>
            void bin_data_(InputIt first_x, InputIt last_x, InputIt first_y, InputIt last_y,
                std::size_t xbins, std::size_t ybins) {
                nbinsx = xbins;
                nbinsy = ybins;
                auto minmax_x = *std::minmax_element(first_x, last_x);
                auto minmax_y = *std::minmax_element(first_y, last_y);
                range_type min_x = std::floor(*minmax_x.first);
                range_type max_x = std::ceil(*minmax_x.second);
                range_type min_y = std::floor(*minmax_y.first);
                range_type max_y = std::ceil(*minmax_y.second);
                xbin_size = (max_x - min_x)/nbinsx;
                ybin_size = (max_y - min_y)/nbinsy;
                range_type bs_x_recip = static_cast<range_type>(1.0 / xbin_size);
                range_type bs_y_recip = static_cast<range_type>(1.0 / ybin_size);
                for (std::size_t i = 0U; i < nbinsx; ++i) {
                    for (std::size_t j = 0U; j < nbinsy; ++j) 
                        rh[std::make_pair(
                            std::make_pair(min_x + i*xbin_size, min_x + (i+1)*xbin_size),
                            std::make_pair(min_y + j*ybin_size, min_y + (j+1)*ybin_size)
                        )] = 0U;
                }
                for (; first_x < last_x || first_y < last_y; ++first_x; ++first_y) {
                    std::size_t bin_x = (*first_x - min_x)*bs_x_recip;
                    std::size_t bin_y = (*first_y - min_y)*bs_y_recip;
                    rh[std::make_pair(
                        std::make_pair(min_x + bin_x*xbin_size, min_x + (bin_x + 1)*xbin_size),
                        std::make_pair(min_y + bin_y*ybin_size, min_y + (bin_y + 1)*ybin_size)
                    )]++;
                }
            }
            rhist_t rh;
            std::size_t nbinsx;
            std::size_t nbinsy;
            range_type xbin_size;
            range_type ybin_size;
        };
        
        template<class RTy>
        ranged_histogram<RTy> marginalise_y(const ranged_histogram_2d<RTy>& hist_2d) {
            std::map<std::pair<RTy, RTy>, std::size_t> marginalised;
            std::size_t i = 0U; std::size_t acc = 0U;
            for (const auto& p : hist_2d) {
                acc += p.second;
                if (!(i % hist_2d.xbins())) {
                    marginalised[p.first.first] = 
                        static_cast<std::size_t>(hist_2d.xbin_width()*acc);
                    acc = 0U;
                }
                ++i;
            }
            return ranged_histogram<RTy>(std::move(marginalised));
        }
    }
}
#endif // !RANGED_HISTOGRAM_H