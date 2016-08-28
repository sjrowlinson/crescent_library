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
	template<class Ty,
		class Generator = std::mt19937,
		class = std::enable_if_t<std::is_integral<Ty>::value>
	> class uniform_int_random_generator {
	public:
		uniform_int_random_generator(const Ty& min = 0, const Ty& max = std::numeric_limits<Ty>::max())
			: eng{ std::random_device{}() }, dist(min, max) {}
		Ty operator()() { return dist(eng); }
		Ty min() const { return dist.min(); }
		Ty max() const { return dist.max(); }
	private:
		Generator eng;
		std::uniform_int_distribution<Ty> dist;
	};

	template<class Ty,
		class Generator = std::mt19937,
		class = std::enable_if_t<std::is_floating_point<Ty>::value>
	> class uniform_fp_random_generator {
	public:
		uniform_fp_random_generator(const Ty& min, const Ty& max)
			: eng{ std::random_device{}() }, dist(min, max) {}
		Ty operator()() { return dist(eng); }
		Ty min() const { return dist.min(); }
		Ty max() const { return dist.max(); }
	private:
		Generator eng;
		std::uniform_real_distribution<Ty> dist;
	};
}



