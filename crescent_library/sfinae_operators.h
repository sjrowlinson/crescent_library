#ifndef SFINAE_OPERATORS_H
#define SFINAE_OPERATORS_H
#include <ostream>

namespace crsc {
	/**
	 * \brief Detail namespace for implementation of `has_insertion_operator` struct.
	 */
	namespace has_insertion_operator_impl {
		typedef char no;
		typedef char yes[2];
		struct any_t {	// any type
			template<typename Ty> any_t(const Ty&);
		};
		no operator<<(const std::ostream&, const any_t&);
		yes& test(std::ostream&);	// overload resolution, operator<< should return its first argument
		no test(no);
		// check for insertion operator implicitly
		template<typename Ty>
		struct has_insertion_operator {
			static std::ostream& os;	// instance of std::ostream
			static const Ty& t;	// instance of type to check
			static const bool value = sizeof(test(os << t)) == sizeof(yes); // check for overloaded operator<<
		};
	}
	/**
	 * \struct has_insertion_operator
	 * \tparam Ty The type to check for overloaded `operator<<`.
	 * \brief Struct to check for an overloaded `operator<<` on any type,
	 *        use via e.g: `std::enable_if_t<has_insertion_operator<Ty>::value>`.
	 */
	template<typename Ty>
	struct has_insertion_operator :
		has_insertion_operator_impl::has_insertion_operator<Ty> {
	};
}

#endif // !SFINAE_OPERATORS_H