#ifndef MARKOV_CHAIN_MONTE_CARLO_H
#define MARKOV_CHAIN_MONTE_CARLO_H
#include <array>
#include <cmath>
#include <functional>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

namespace crsc {
	namespace mc {
		/**
		 * \brief Performs a Markov Chain Monte-Carlo analysis in `Dims` dimensions on data in the
		 *        range `[first, last)` using the Metropolis-Hastings algorithm to decide on jump
		 *        sampling conditions. The proposal density used for the jump distributions is 
         *        a normal distribution, hence the npd acronym.
		 *
		 * This function performs random sampling on some functional posterior form `f` using the
		 * `std::mt19937` engine seeded with a `std::seed_seq` of 8 `std::random_device` instances.
		 *
		 * \tparam Ty Type of data stored in data range, must satisfy `std::is_floating_point<Ty>::value`.
		 * \tparam Dims Number of dimensions.
		 * \tparam InputIt `InputIterator` type.
		 * \tparam Args Types of arguments to forward to function `f`.
		 * \param first Beginning of data range.
		 * \param last End of data range.
		 * \param init Initial values for each variable.
		 * \param prior Priors on each variable, defining the bounds within which the posterior is non-zero.
		 * \param jsigma Normal distribution standard deviation for the jumps on each variable.
		 * \param samples Number of samples to perform.
		 * \param f Functional form of posterior pdf.
		 * \param f_args Optional arguments to forward to function `f`.
		 * \return A `std::vector` containing the `Dims` dimensional "posterior pdf" data.
		 */
		template<class Ty,
			std::size_t Dims,
			class InputIt,
			class... Args,
			class = std::enable_if_t<std::is_floating_point<Ty>::value>
		> std::vector<std::array<Ty, Dims>> mcmc_metropolis_hastings_npd(InputIt first, InputIt last, 
			const std::array<Ty, Dims>& init, const std::array<std::pair<Ty, Ty>, Dims>& prior, 
			const std::array<Ty, Dims>& jsigma, std::size_t samples, 
			std::function<Ty(InputIt, InputIt, const std::array<Ty, Dims>&, Args&&...)> f, Args&&... f_args) {
			std::vector<std::array<Ty, Dims>> posterior; posterior.reserve(samples); // pre-allocate for speed
			std::random_device rd;
			std::seed_seq seed{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
			std::mt19937 mt_eng(seed); // mersenne-twister engine for prng
			std::uniform_real_distribution<Ty> pdist(Ty(), static_cast<Ty>(1.0)); // generate probabilities
			std::array<std::normal_distribution<Ty>, Dims> jdist_arr;
			// jump distributions for each variable
			for (std::size_t i = 0U; i < Dims; ++i) jdist_arr[i] = std::normal_distribution<Ty>(Ty(), jsigma[i]);
			std::array<Ty, Dims> curr_state = init;
			std::array<Ty, Dims> prop_state = init;
			std::array<Ty, Dims> jump_arr;
			Ty p_curr = Ty(); // posterior prob for current state
			Ty p_prop = Ty(); // posterior prob for proposed state
			Ty ratio = Ty();
			bool skip = false; // skip to next loop iter flag
			// monte-carlo loop
			for (std::size_t i = 0U; i < samples; ++i) {
				skip = false; // reset skip status
				// compute current posterior
				p_curr = f(first, last, curr_state, std::forward<Args>(f_args)...);
				for (std::size_t j = 0U; j < Dims; ++j) {
					jump_arr[j] = jdist_arr[j](mt_eng); // generate random jump
					prop_state[j] = curr_state[j] + jdist_arr[j](mt_eng); // translate proposed state
					// check for proposed state falling outside any prior boundary
					if (prop_state[j] > prior[j].second || prop_state[j] < prior[j].first) {
						p_prop = Ty();
						skip = true;
						break;
					}
				}
				if (skip) { posterior.push_back(curr_state); continue; } // if proposed fell outside prior, skip to next loop iter
				// compute proposed posterior
				else p_prop = f(first, last, prop_state, std::forward<Args>(f_args)...);
				ratio = p_prop / p_curr;
				// metropolis-hasting algorithm criterion
				if (ratio >= static_cast<Ty>(1.0) || ratio > pdist(mt_eng)) curr_state = prop_state;
				posterior.push_back(curr_state); // push current state to return vector
			}
			return posterior;
		}
	}
}
#endif // !MARKOV_CHAIN_MONTE_CARLO_H

