#include <algorithm>
#include <deque>
#include <cmath>
#include <complex>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace crsc {
#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H
	/**
	 * \class polynomial
	 *
	 * \brief The `crsc::polynomial` class is a container adapter giving the functionality of a mathematical
	 *        polynomial construct - specifically, a data structure storing coefficients of orders of a single
	 *        variable polynomial equation through the abstraction of a `std::deque`.
	 *
	 * Coefficients are stored in terms of the sequentially increasing orders, i.e:
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
		 * \brief Constructs a polynomial of zero order.
		 */
		polynomial() 
			: coeffs() {}
		/**
		 * \brief Constructs a polynomial of order `degree` where the coefficient of each 
		 *        term is the default-inserted value of `coefficient_type`.
		 * \param degree Order of polynomial.
		 */
		explicit polynomial(size_type degree) 
			: coeffs(degree) {}
		/**
		 * \brief Constructs a polynomial of order `degree` where the coefficient of each
		 *        term is a copy of the value `value`.
		 * \param degree Order of polynomial.
		 * \param value Value to initialise all coefficients with.
		 */
		explicit polynomial(size_type degree, const coefficient_type& value)
			: coeffs(degree, value) {}
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
		/**
		 * \brief Copy constructor. Constructs a polynomial with a copy of the contents
		 *        of `other`.
		 * \param other `polynomial` instance to use as data source.
		 */
		polynomial(const polynomial& other)
			: coeffs(other.coeffs) {}
		/**
		 * \brief Move constructor. Constructs a polynomial with the contents of `other`
		 *        via move-semantics.
		 * \param other `polynimial` instance to use as data source.
		 */
		polynomial(polynomial&& other)
			: coeffs(std::move(other.coeffs)) {}
		/**
		 * \brief Copy-assignment operator. Replaces the contents of `this` polynomial 
		 *        with a copy of the contents of `other`.
		 * \param other `polynomial` instance to use as data source.
		 * \return `*this`.
		 */
		polynomial& operator=(const polynomial& other) {
			if (this != &other)
				coeffs = other.coeffs;
			return *this;
		}
		/**
		 * \brief Move-assignment operator. Replaces the contents of `this` polynomial
		 *        with the contents of `other` via move-semantics.
		 * \param other `polynomial` instance to use as data source.
		 * \return `*this`. 
		 */
		polynomial& operator=(polynomial&& other) {
			if (this != &other)
				coeffs = std::move(other.coeffs);
			return *this;
		}
		// PROPERTIES
		/**
		 * \brief Returns the order of the polynomial.
		 * \return Order of the polynomial.
		 * \complexity Constant.
		 */
		size_type order() const noexcept { return coeffs.size(); }
		/**
		 * \brief Returns `true` if the polynomial is zero order (no elements), `false` otherwise.
		 * \return `true` if `order()` of polynomial is zero, `false` otherwise.
		 */
		bool zero_order() const noexcept { return coeffs.empty(); }
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
		/**
		 * \brief Evaluates the polynomial at a given value `val`.
		 * \param val Point at which to evaluate the polynomial.
		 * \return Value of polynomial at `val`.
		 */
		coefficient_type evaluate_at(const coefficient_type& val) {
			coefficient_type eval = coefficient_type();
			size_type order_count = 0U;
			for (const auto& el : coeffs)
				eval += el*std::pow(val, order_count++);
			return eval;
		}
		/**
		 * \brief Computes the nth derivative of the polynomial and sets
		 *        `*this` to the result.
		 * \param n Number of times to differentiate.
		 */
		void nth_derivative(size_type n) {
			if (n > order()) { coeffs.resize(1U); coeffs[0] = coefficient_type(); }
			for (size_type i = 0; i < n; ++i) coeffs.pop_front();
			std::for_each(coeffs.begin(), coeffs.end(), [](auto& coeff) { coeff *= factorial(n); });
		}
		/**
		 * \brief Computes the nth indefinite integral of the polynomial and
		 *        sets `*this` to the result.
		 * \param n Number of times to integrate.
		 */
		void nth_indefinite_integral(size_type n) {

		}
		// MODIFIERS
		/**
		 * \brief Adds an extra order to the polynomial, where the coefficient of this order
		 *        is a copy of the value `val`.
		 * \param val Value of coefficient of extra order.
		 */
		void increment_order(const coefficient_type& val) { coeffs.push_back(val); }
		/**
		 * \brief Adds an extra order to the polynomial, where the coefficient of this order
		 *        is the moved value `val`.
		 * \param val Value of coefficient of extra order.
		 */
		void increment_order(coefficient_type&& val) { coeffs.push_back(std::move(val)); }
		/**
		 * \brief Reduces the order of the polynomial by 1, removing the last element
		 *        from the coefficient container.
		 */
		void decrement_order() { coeffs.pop_back(); }
	private:
		std::deque<coefficient_type> coeffs;
		template<class T,
			class = std::enable_if_t<std::is_integral<T>::value>
		> T factorial(T n) {
			T rtn = static_cast<T>(1);
			for (T i = static_cast<T>(1); i <= n; ++i) rtn *= i;
			return rtn;
		}
	};
	template<class Ty = double>
	Ty evaluate_polynomial(const polynomial<Ty>& pn, Ty val) {
		return pn.evaluate_at(val);
	}
	template<class Ty = double>
	polynomial<Ty> compute_nth_derivative(const polynomial<Ty>& pn, std::size_t n) {
		polynomial<Ty> pn_cpy(pn);
		pn_cpy.nth_derivative(n);
		return pn_cpy;
	}
	template<class Ty = double>
	polynomial<Ty> compute_nth_indefinite_integral(const polynomial<Ty>& pn, std::size_t n) {
		polynomial<Ty> pn_cpy(pn);
		pn_cpy.nth_indefinite_integral(n);
		return pn_cpy;
	}
	template<class Ty = double>
	std::string parse_polynomial_to_string(const polynomial<Ty> pn) {
		std::string rtn = ""s;
		for (std::size_t i = 0; i < pn.order(); ++i) // TODO: replace with iterators once implemented
			rtn += std::to_string(pn[i]) + (i ? ("x" + (i > 1 ? ("^" + std::to_string(i)) : "")) : "");
		return rtn;
	}
	template<class Ty = double>
	polynomial<Ty> parse_string_to_polynomial(const std::string& pstr) {
		for (const auto& c : pstr) {
			
		}
	}
#endif // !POLYNOMIAL_H
}