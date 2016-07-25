#ifndef SFINAE_OPERATORS_H
#define SFINAE_OPERATORS_H
#include <ostream>

namespace crsc {
	// implementation to check a type for overloaded operator<<
	namespace has_insertion_operator_impl {
		typedef char no;
		typedef char yes[2];
		struct any_t {	// any type
			template<typename _Ty> any_t(const _Ty&);
		};
		no operator<<(const std::ostream&, const any_t&);
		yes& test(std::ostream&);	// overload resolution, operator<< should return its first argument
		no test(no);
		// check for insertion operator implicitly
		template<typename _Ty>
		struct has_insertion_operator {
			static std::ostream& os;	// instance of std::ostream
			static const _Ty& t;	// instance of type to check
			static const bool value = sizeof(test(os << t)) == sizeof(yes); // check for overloaded operator<<
		};
	}
	// struct to be used to check for overloaded operator<< 
	// e.g: std::enable_if_t<has_insertion_operator<_Type>::value>
	template<typename _Ty>
	struct has_insertion_operator :
		has_insertion_operator_impl::has_insertion_operator<_Ty> {
	};
}

#endif // !SFINAE_OPERATORS_H