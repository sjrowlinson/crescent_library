#include <algorithm>
#include <cmath>
#include <complex>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace crsc {
#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H
	/**
	 * \class polynomial
	 *
	 * \brief Represents a mathematical-polynomial construct of a single variable where coefficients of each sequentially
	 *        increasing order are stored contiguously.
	 *
	 * Coefficients are stored contiguously in terms of the sequentially increasing orders, i.e:
	 *	
	 *	P(x) = c[0] + c[1]x + c[2]x^2 + ... + c[order-1]x^{order-1}.
	 *
	 * This class provides constant-time access to the `order` of the polynomial, as well as to each coefficient. Methods are
	 * provided to evaluate the polynomial at specific points (`evaluate_at`) and compute nth derivatives (`nth_derivative`).
	 */
	template<class Ty = double>
	class polynomial {
	public:
		typedef Ty coefficient_type;
		typedef std::size_t size_type;
		// CONSTRUCTION/ASSIGNMENT
		/**
		 * \brief Constructs a polynomial using a range `[first, last)` of coefficients.
		 * \param first Beginning of coefficient range (marks order 0 coeff.)
		 * \param last End of coefficient range (marks order `std::distance(first,last)` coeff.)
		 * \complexity Linear in `std::distance` between `first` and `last`.
		 */
		template<class InputIt>
		polynomial(InputIt first, InputIt last) 
			: coeffs(first, last) {}
		/**
		 * \brief Constructs a polynomial using an `std::initializer_list` of coefficients.
		 * \param ilist `std::initializer_list` containing coefficient values.
		 * \complexity Linear in `ilist.size()`.
		 */
		polynomial(std::initializer_list<coefficient_type> ilist)
			: coeffs(ilist) {}
		// PROPERTIES
		/**
		 * \brief Returns the order of the polynomial.
		 * \return Order of the polynomial.
		 * \complexity Constant.
		 */
		size_type order() const noexcept { return coeffs.size(); }
		// COEFFICIENT VALUE ACCESS
		const coefficient_type& operator[](size_type n) const { return coeffs[n]; }
		coefficient_type& operator[](size_type n) { return coeffs[n]; }
		const coefficient_type& coefficient_at_order(size_type n) const {
			if (!(n < coeffs.size())) throw std::out_of_range("order out of bounds.");
			return coeffs[n];
		}
		coefficient_type& coefficient_at_order(size_type n) {
			if (!(n < coeffs.size())) throw std::out_of_range("order out of bounds.");
			return coeffs[n];
		}
		// EVALUATORS
		coefficient_type evaluate_at(const coefficient_type& val) {
			coefficient_type eval = coefficient_type();
			size_type order_count = 0U;
			for (const auto& el : coeffs)
				eval += el*std::pow(val, order_count++);
			return eval;
		}
		polynomial& nth_derivative(size_type n) {
			int order_count = 0 - n;
			std::for_each(coeffs.begin(), coeffs.end(), [&order_count](auto& coeff) { 
				coeff *= static_cast<coefficient_type>((order_count > 0) ? factorial(order_count++) : 0); 
			});
			return *this;
		}
		polynomial& integral(size_type n) {

		}
	private:
		std::vector<coefficient_type> coeffs;
		size_type factorial(size_type n) {
			size_type rtn = 1U;
			for (size_type i = 1; i <= n; ++i) rtn *= i;
			return rtn;
		}
	};
#endif // !POLYNOMIAL_H



}