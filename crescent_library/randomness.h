#ifndef RANDOM_PROBABILITY_GENERATOR_H
#define RANDOM_PROBABILITY_GENERATOR_H
#include <random>

template<class Ty = double,
	class Generator = std::mt19937,
	class Distribution = std::uniform_real_distribution<Ty>,
	class = std::enable_if_t<std::is_floating_point<Ty>::value>
> class random_probability_generator {
public:
	random_probability_generator()
		: eng{ std::random_device{}() }, prob_dist{0.0, 1.0} {}
	Ty next() { return prob_dist(eng); }
private:
	Generator eng;
	Distribution prob_dist;
};

#endif // !RANDOM_PROBABILITY_GENERATOR_H

