#include <complex>
#include <random>

namespace crsc {
#ifndef RANDOM_NUMBER_GENERATOR_H
#define RANDOM_NUMBER_GENERATOR_H
	/**
	 * \class random_number_generator
	 *
	 * \brief Pseudo-random number generator for random values over a specified `Distribution`
	 *        using a given `Generator` engine.
	 *
	 * A convenience wrapper around a generator engine and random number distribution used for 
	 * generating random values quickly and simply. Any pre-defined generator from the C++ `<random>`
	 * header may be used as the `Generator` type-param and any distribution from this header
	 * may be used for the `Distribution` type-param. The next value in the random distribution
	 * is generated via a call to `random_number_generator::operator()`. Resetting the internal
	 * state of the distribution such that the next generating call is not dependent upon the last
	 * call is achieved via a call to `random_number_generator::reset_distribution_state()`.
	 *
	 * Note that the type `Ty` (referenced as `result_type` in the class API) must match the type of
	 * distribution `Distribution` (referenced as `distribution_type` in the class API) used, e.g.
	 * if the `result_type` is `int` then it is undefined behaviour to use a distribution type 
	 * intended for floating point types.
	 *
	 * \tparam Ty The type of the values to generate, must satisfy `std::is_arithmetic<Ty>`. Defaults
	 *         to the integral type `int`.
	 * \tparam Generator The type of the generator engine to use for pseudo-random generation, must
	 *         meet the requirement of `UniformRandomBitGenerator` (see C++ Concepts). Defaults to
	 *         the engine type `std::mt19937`.
	 * \tparam Distribution The type of distribution over which to calculate the random numbers. The
	 *         value type of the distribution must match the value type `Ty` of this class. Must meet
	 *         the requirement of `RandomNumberDistribution` (see C++ Conecpts). Defaults to the
	 *         distribution type `std::uniform_int_distribution<Ty>`.
	 */
	template<class Ty = int,
		class Generator = std::mt19937,
		class Distribution = std::uniform_int_distribution<Ty>,
		class = std::enable_if_t<std::is_arithmetic<Ty>::value>
	> class random_number_generator {
	public:
		// PUBLIC API TYPE DEFINITIONS
		typedef Ty result_type;
		typedef Generator generator_type;
		typedef Distribution distribution_type;
		// CONSTRUCTION/ASSIGNMENT
		/**
		 * \brief Move constructs the generator with the values of the engine `_eng` and the
		 *        distribution `_dist`. This is also the default constructor.
		 *
		 * \param _eng Generator engine to use.
		 * \param _dist Distribution for random numbers.
		 */
		explicit random_number_generator(Generator&& _eng = Generator{std::random_device{}()}, Distribution&& _dist = Distribution())
			: eng(std::move(_eng)), dist(std::move(_dist)) {}
		/**
		 * \brief Constructs the generator with a copy of the values of the engine `_eng` and
		 *        the distribution `_dist`.
		 *
		 * \param _eng Generator engine to use.
		 * \param _dist Distribution for random numbers.
		 */
		explicit random_number_generator(const Generator& _eng, const Distribution& _dist)
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
		 * \brief Copy-assignment operator. Replaces the generator with a copy of the fields of `other`.
		 * \param other `random_number_generator` instance to use as data source.
		 * \return `*this`.
		 */
		random_number_generator& operator=(const random_number_generator& other) {
			if (this != &other)
				random_number_generator(other).swap(*this); // copy-and-swap
			return *this;
		}
		/**
		 * \brief Move-assignment operator. Replaces the generator with the field of `other` 
		 *        using move-semantics.
		 * \param other `random_number_generator` instance to use as data source.
		 * \return `*this`.
		 */
		random_number_generator& operator=(random_number_generator&& other) {
			if (this != &other)
				swap(*this, std::move(other));
			return *this;
		}
		// GENERATING OPERATOR()
		/**
		 * \brief Generates the next random number in the distribution.
		 *
		 * \return The generated random number.
		 */
		result_type operator()() { return dist(eng); }
		// GENERATOR AND DISTRIBUTION OBJECT ACCESS
		/**
		 * \brief Returns a copy of the underlying generator engine.
		 * \return A copy of the underlying engine used for random number generation.
		 */
		generator_type get_generator() const noexcept { return eng; }
		/**
		 * \brief Returns a copy of the underlying distribution.
		 * \return A copy of the underlying distribution over which the random
		 *         numbers are generated.
		 */
		distribution_type get_distribution() const noexcept { return dist; }
		// PROPERTIES
		/**
		 * \brief Returns the minimum potentially generated value.
		 * \return The minimum value potentially generated by the underlying distribution.
		 */
		result_type min() const { return dist.min(); }
		/**
		 * \brief Returns the maximum potentially generated value.
		 * \return The maximum value potentially generated by the underlying distribution.
		 */
		result_type max() const { return dist.max(); }
		// MODIFIERS
		/**
		 * \brief Resets the internal state of the underlying distribution object. After calling this function,
		 *        the next call to `operator()` on the generator will not be dependent upon previous calls
		 *        to `operator()`.
		 */
		void reset_distribution_state() { dist.reset(); }
		/**
		 * \brief Exchanges the fields of the generator with those of `other`.
		 *
		 * \param other `random_number_generator` object to swap with.
		 */
		void swap(random_number_generator& other) {
			std::swap(eng, other.eng);
			std::swap(dist, other.dist);
		}
		/**
		 * \brief Exchanges the fields of the generator `lhs` with those of `rhs`.
		 *
		 * \param lhs `random_number_generator` object to swap with `rhs`.
		 * \param rhs `random_number_generator` object to swap with `lhs`.
		 */
		static void swap(random_number_generator& lhs, random_number_generator& rhs) { lhs.swap(rhs); }
	private:
		generator_type eng;
		distribution_type dist;
	};
#endif // !RANDOM_NUMBER_GENERATOR_H
#ifndef UNIFORM_RANDOM_PROBABILITY_GENERATOR_H
#define UNIFORM_RANDOM_PROBABILITY_GENERATOR_H
	/**
	 * \class uniform_random_probability_generator
	 *
	 * \brief Pseudo-random number generator for random floating point values distributed
	 *        uniformly over the range [0.0, 1.0] using a given `Generator` engine.
	 *
	 * A convenience wrapper around a template specialisation of `crsc::random_number_generator` providing
	 * a class to produce uniformly distributed floating point values in the range [0.0, 1.0]. Any pre-defined
	 * generator from the C++ `<random>` header may be used as the `Generator` type-param. The next value in
	 * the random distribution is generated via a call to `uniform_random_probability_generator::operator()`. 
	 * Resetting the internal state of the distribution such that the next generating call is not dependent upon
	 * the last call is achieved via a call to `uniform_random_probability_generator::reset_distribution_state()`.
	 *
	 * \tparam FloatType The type of the probabilities to generator, must satisfy `std::is_floating_point<FloatType>`.
	 *         Defaults to the type `double`.
	 * \tparam Generator The type of the generator engine to use for pseudo-random generation, must
	 *         meet the requirement of `UniformRandomBitGenerator` (see C++ Concepts). Defaults to
	 *         the engine type `std::mt19937`. 
	 */
	template<class FloatType = double,
		class Generator = std::mt19937,
		class = std::enable_if_t<std::is_floating_point<FloatType>::value>
	> class uniform_random_probability_generator {
		typedef random_number_generator<FloatType, Generator, std::uniform_real_distribution<FloatType>> uniform_pr_gen;
	public:
		// PUBLIC API TYPE DEFINITIONS
		typedef typename uniform_pr_gen::result_type result_type;
		typedef typename uniform_pr_gen::generator_type generator_type;
		typedef typename uniform_pr_gen::distribution_type distribution_type;
		// CONSTRUCTION/ASSIGNMENT
		explicit uniform_random_probability_generator(Generator&& engine = Generator{std::random_device{}()})
			: generator(std::move(engine)) {}
		explicit uniform_random_probability_generator(const Generator& engine)
			: generator(engine, distribution_type()) {}
		uniform_random_probability_generator(const uniform_random_probability_generator& other)
			: generator(other.generator) {}
		uniform_random_probability_generator(uniform_random_probability_generator&& other)
			: generator(std::move(other.generator)) {}
		uniform_random_probability_generator& operator=(const uniform_random_probability_generator& other) {
			if (this != &other)
				generator = other.generator;
			return *this;
		}
		uniform_random_probability_generator& operator=(uniform_random_probability_generator&& other) {
			if (this != &other)
				generator = std::move(other.generator);
			return *this;
		}
		// GENERATING OPERATOR()
		result_type operator()() { return generator(); }
		// GENERATOR AND DISTRIBUTION OBJECT ACCESS
		generator_type get_generator() const noexcept { return generator.get_generator(); }
		distribution_type get_distribution() const noexcept { return generator.get_distribution(); }
		// PROPERTIES
		constexpr result_type min() const { return generator.min(); }
		constexpr result_type max() const { return generator.max(); }
		// MODIFIERS
		void reset_distribution_state() { generator.reset_distribution_state(); }
		void swap(uniform_random_probability_generator& other) { generator.swap(other.generator); }
		static void swap(uniform_random_probability_generator& lhs, uniform_random_probability_generator& rhs) { lhs.swap(rhs); }
	private:
		uniform_pr_gen generator;
	};
#endif // !UNIFORM_RANDOM_PROBABILITY_GENERATOR_H
#ifndef RANDOM_COMPLEX_GENERATOR_H
#define RANDOM_COMPLEX_GENERATOR_H
	/**
	 * \class random_complex_generator
	 *
	 * \brief Generator for random complex numbers in `std::complex` object form over a specified `Distribution`
	 *        using a given `Generator` engine.
	 *
	 * A convenience wrapper around a generator engine and random number distribution used for generating random complex
	 * numbers quickly and simply. Any pre-defined generator from the C++ `<random>` header may be used as the `Generator`
	 * type-param and any distribution from this header may be used for the `Distribution` type-param. The next value in
	 * the random distribution is generated via a call to `random_complex_generator::operator(bool)` where the `bool` param
	 * indicates whether the random complex number generated shall have equal Real and Imaginary parts. Resetting the internal
	 * state of the distribution such that the next generating call is not dependent upon the last call is achieved via a 
	 * call to `random_complex_generator::reset_distribution_state()`.
	 *
	 * Note that the type `Ty` (referenced as `real_imaginary_type` in the class API) must match the type of distribution
	 * `Distribution` (referenced as `distribution_type` in the class API) used, e.g. if the `real_imaginary_type` is `int`
	 * then it is undefined behaviour to use a distribution type intended for floating point types.
	 *
	 * \tparam Ty The type of the real and imaginary parts of the complex number, must
	 *         satisfy `std::is_arithmetic<Ty>`. Defaults to the floating type `double`.
	 * \tparam Generator The type of the generator engine to use for pseudo-random generation,
	 *         must meet the requirement of `UniformRandomBitGenerator` (see C++ Concepts). Defaults
	 *         to the engine type `std::mt19937`.
	 * \tparam Distribution The type of distribution over which to calculate the random numbers. The
	 *         value type of the distribution must match the value type `Ty` of this class. Must meet
	 *         the requirement of `RandomNumberDistribution` (see C++ Conecpts). Defaults to the
	 *         distribution type `std::uniform_real_distribution<Ty>`.
	 */
	template<class Ty = double,
		class Generator = std::mt19937,
		class Distribution = std::uniform_real_distribution<Ty>,
		class = std::enable_if_t<std::is_arithmetic<Ty>::value>
	> class random_complex_generator {
	public:
		// PUBLIC API TYPE DEFINITIONS
		typedef Ty real_imaginary_type;
		typedef typename std::complex<Ty> result_type;
		typedef Generator generator_type;
		typedef Distribution distribution_type;
		// CONSTRUCTION/ASSIGNMENT
		/**
		 * \brief Move constructs the generator with the values of the engine `_eng` and the
		 *        distribution `_dist`. This is also the default constructor.
		 *
		 * \param _eng Generator engine to use.
		 * \param _dist Distribution for random complex numbers.
		 */
		explicit random_complex_generator(Generator&& _eng = Generator{std::random_device{}()}, Distribution&& _dist = Distribution())
			: eng(std::move(_eng)), dist(std::move(_dist)) {}
		/**
		 * \brief Constructs the generator with a copy of the values of the engine `_eng` and
		 *        the distribution `_dist`.
		 *
		 * \param _eng Generator engine to use.
		 * \param _dist Distribution for random complex numbers.
		 */
		explicit random_complex_generator(const Generator& _eng, const Distribution& _dist)
			: eng(_eng), dist(_dist) {}
		/**
		 * \brief Copy constructor. Constructs the generator with a copy of the fields of `other`.
		 *
		 * \param other `random_complex_generator` instance to use as data source.
		 */
		random_complex_generator(const random_complex_generator& other)
			: eng(other.eng), dist(other.dist) {}
		/**
		 * \brief Move constructor. Constructs the generator with the fields of `other` using
		 *        move-semantics such that `other` is left in a valid but unspecified state.
		 *
		 * \param other `random_complex_generator` instance to use as data source.
		 */
		random_complex_generator(random_complex_generator&& other)
			: eng(std::move(other.eng)), dist(std::move(other.dist)) {}
		// GENERATING OPERATOR()
		/**
		 * \brief Generates the next random complex number in the distribution.
		 *
		 * \param is_equal_re_im Determines whether the real and imaginary
		 *        parts of the generated complex number will be equal or not.
		 * \return The generated random complex number.
		 */
		result_type operator()(bool is_equal_re_im = false) {
			if (!is_equal_re_im) return result_type(dist(eng), dist(eng));
			real_imaginary_type re_im = dist(eng);
			return result_type(re_im, re_im);
		}
		// GENERATOR AND DISTRIBUTION OBJECT ACCESS
		/**
		 * \brief Returns a copy of the underlying generator engine.
		 * \return A copy of the underlying engine used for random number generation.
		 */
		generator_type get_generator() const noexcept { return eng; }
		/**
		 * \brief Returns a copy of the underlying distribution.
		 * \return A copy of the underlying distribution over which the random
		 *         numbers are generated.
		 */
		distribution_type get_distribution() const noexcept { return dist; }
		// PROPERTIES
		/**
		 * \brief Returns the minimum potentially generated complex number where both the
		 *        real and imaginary parts are the minimum of the distribution.
		 * \return The minimum `std::complex` number potentially generated by the underlying distribution.
		 */
		result_type min() const { return result_type(dist.min(), dist.min()); }
		/**
		 * \brief Returns the maximum potentially generated complex number where both the
		 *        real and imaginary parts are the maximum of the distribution.
		 * \return The maximum `std::complex` number potentially generated by the underlying distribution.
		 */
		result_type max() const { return result_type(dist.max(), dist.max()); }
		// MODIFIERS
		/**
		 * \brief Resets the internal state of the underlying distribution object. After calling this function,
		 *        the next call to `operator()` on the generator will not be dependent upon previous calls
		 *        to `operator()`.
		 */
		void reset_distribution_state() { dist.reset(); }
	private:
		generator_type eng;
		distribution_type dist;
	};
#endif // !RANDOM_COMPLEX_GENERATOR_H
}


