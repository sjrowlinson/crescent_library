#include <complex>
#include <random>

namespace crsc {
#ifndef RANDOM_PROBABILITY_GENERATOR_H
#define RANDOM_PROBABILITY_GENERATOR_H
	/**
	 * \class uniform_random_probability_generator
	 *
	 * \brief Generator for uniform random probability values (i.e. floating point values
	 *        in the range [0.0, 1.0]) using a given `Generator`.
	 *
	 * \tparam Ty The type of the probabilities to generate, must satisfy `std::is_floating_point<Ty>`.
	 * \tparam Generator The type of the generator engine to use for pseudo-random generation.
	 */
	template<class Ty = double,
		class Generator = std::mt19937,
		class = std::enable_if_t<std::is_floating_point<Ty>::value>
	> class uniform_random_probability_generator {
	public:
		uniform_random_probability_generator()
			: eng{ std::random_device{}() }, prob_dist(0.0, 1.0) {}
		Ty operator()() { return prob_dist(eng); }
	private:
		Generator eng;
		std::uniform_real_distribution<Ty> prob_dist;
	};
#endif // !RANDOM_PROBABILITY_GENERATOR_H
#ifndef RANDOM_NUMBER_GENERATOR_H
#define RANDOM_NUMBER_GENERATOR_H
	/**
	 * \class random_number_generator
	 *
	 * \brief Pseudo random number generator for random values over a specified `Distribution`
	 *        using a given `Generator` engine.
	 *
	 * \tparam Ty The type of the values to generate, must satisfy `std::is_arithmetic<Ty>`. Defaults
	 *         to the integral type `int`.
	 * \tparam Generator The type of the generator engine to use for pseudo-random generation, must
	 *         meet the requirement of `UniformRandomBitGenerator` (see C++ Concepts). Defaults to
	 *         the engine type `std::mt19937`.
	 * \tparam Distribution The type of distribution over which to calculate the random integers. The
	 *         value type of the distribution must match the value type `Ty` of this class. Defaults
	 *         to the distribution type `std::uniform_int_distribution<Ty>`.
	 */
	template<class Ty = int,
		class Generator = std::mt19937,
		class Distribution = std::uniform_int_distribution<Ty>,
		class = std::enable_if_t<std::is_arithmetic<Ty>::value>
	> class random_number_generator {
	public:
		/**
		 * \brief Move constructs the generator with the values of the engine `_eng` and the
		 *        distribution `_dist`. This is also the default constructor.
		 *
		 * \param _eng Generator engine to use.
		 * \param _dist Distribution for random numbers.
		 */
		random_number_generator(Generator&& _eng = Generator{std::random_device{}()}, Distribution&& _dist = Distribution())
			: eng(std::move(_eng)), dist(std::move(_dist)) {}
		/**
		 * \brief Constructs the generator with a copy of the values of the engine `_eng` and
		 *        the distribution `_dist`.
		 *
		 * \param _eng Generator engine to use.
		 * \param _dist Distribution for random numbers.
		 */
		random_number_generator(const Generator& _eng, const Distribution& _dist)
			: eng(_eng), dist(_dist) {}
		/**
		 * \brief Copy constructor. Constructs the generator with a copy of the fields of `other`.
		 *
		 * \param other `random_number_generator` instance to use as data source.
		 */
		random_number_generator(const random_number_generator& other)
			: eng(other.eng), dist(other.dist) {}
		/**
		 * \brief Move constructor. Constructs the generator with the fields of `other` using
		 *        move-semantics such that `other` is left in a valid but unspecified state.
		 *
		 * \param other `random_number_generator` instance to use as data source.
		 */
		random_number_generator(random_number_generator&& other)
			: eng(std::move(other.eng)), dist(std::move(other.dist)) {}
		/**
		 * \brief Generates the next random number in the distribution.
		 *
		 * \return The generated random number.
		 */
		Ty operator()() { return dist(eng); }
	private:
		Generator eng;
		Distribution dist;
	};
#endif // !RANDOM_NUMBER_GENERATOR_H
#ifndef RANDOM_COMPLEX_GENERATOR_H
#define RANDOM_COMPLEX_GENERATOR_H
	/**
	 * \class random_complex_generator
	 *
	 * \brief Generator for random complex numbers in `std::complex` object form.
	 *
	 * \tparam Ty The type of the real and imaginary parts of the complex number.
	 * \tparam Generator The type of the generator engine to use for pseudo-random generation.
	 * \tparam Distribution The type of distribution over which to calculate the random numbers.
	 */
	template<class Ty = double,
		class Generator = std::mt19937,
		class Distribution = std::uniform_real_distribution<Ty>
	> class random_complex_generator {
	public:
		random_complex_generator(Generator&& _eng = Generator{std::random_device{}()}, Distribution&& _dist = Distribution())
			: eng(std::move(_eng)), dist(std::move(_dist)) {}
		random_complex_generator(const Generator& _eng, const Distribution& _dist)
			: eng(_eng), dist(_dist) {}
		random_complex_generator(const random_complex_generator& other)
			: eng(other.eng), dist(other.dist) {}
		random_complex_generator(random_complex_generator&& other)
			: eng(std::move(other.eng)), dist(std::move(other.dist)) {}
		std::complex<Ty> operator()() { return std::complex<Ty>(dist(eng), dist(eng)); }
	private:
		Generator eng;
		Distribution dist;
	};
#endif // !RANDOM_COMPLEX_GENERATOR_H
}



