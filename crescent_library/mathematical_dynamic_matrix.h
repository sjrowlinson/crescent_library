#ifndef MATHEMATICAL_DYNAMIC_MATRIX_H
#define MATHEMATICAL_DYNAMIC_MATRIX_H
#include "dynamic_matrix.h"

namespace crsc {
	template<typename Ty, 
		class Allocator = std::allocator<Ty>,
		class = std::enable_if_t<std::is_arithmetic<Ty>::value>
	> class mathematical_dynamic_matrix;
	template<typename Ty,
		class Allocator = std::allocator<Ty>
	> void swap(const mathematical_dynamic_matrix<Ty, Allocator>& lhs, const mathematical_dynamic_matrix<Ty, Allocator>& rhs) {
		lhs.swap(rhs);
	}
	template<typename Ty,
		class Allocator = std::allocator<Ty>,
		class = std::enable_if_t<std::is_arithmetic<Ty>::value>
	> class mathematical_dynamic_matrix {
		typedef typename crsc::dynamic_matrix<Ty, Allocator> matrix_type;
	public:
		// PUBLIC API TYPE DEFINITIONS
		typedef typename matrix_type::value_type value_type;
		typedef typename matrix_type::reference reference;
		typedef typename matrix_type::const_reference const_reference;
		typedef typename matrix_type::pointer pointer;
		typedef typename matrix_type::const_pointer const_pointer;
		typedef typename matrix_type::size_type size_type;
		typedef typename matrix_type::difference_type difference_type;
		typedef typename matrix_type::allocator_type allocator_type;
		typedef typename matrix_type::iterator iterator;
		typedef typename matrix_type::const_iterator const_iterator;
		typedef typename matrix_type::reverse_iterator reverse_iterator;
		typedef typename matrix_type::const_reverse_iterator const_reverse_iterator;
		// CONSTRUCTION/ASSIGNMENT
		mathematical_dynamic_matrix() : mathematical_dynamic_matrix(Allocator()) {}
		explicit mathematical_dynamic_matrix(const Allocator& alloc)
			: mtx(alloc) {}
		explicit mathematical_dynamic_matrix(size_type rows, size_type cols, const Allocator& alloc = Allocator())
			: mtx(rows, cols, alloc) {}
		explicit mathematical_dynamic_matrix(size_type rows, size_type cols, const value_type& val, const Allocator& alloc = Allocator())
			: mtx(rows, cols, val, alloc) {}
		explicit mathematical_dynamic_matrix(value_type** arr_2d, size_type rows, size_type cols, const Allocator& alloc = Allocator())
			: mtx(arr_2d, rows, cols, alloc) {}
		mathematical_dynamic_matrix(const mathematical_dynamic_matrix& other)
			: mtx(other.mtx) {}
		mathematical_dynamic_matrix(const mathematical_dynamic_matrix& other, const Allocator& alloc)
			: mtx(other.mtx, alloc) {}
		mathematical_dynamic_matrix(mathematical_dynamic_matrix&& other) 
			: mtx(std::move(other.mtx)) {}
		mathematical_dynamic_matrix(mathematical_dynamic_matrix&& other, const Allocator& alloc) 
			: mtx(std::move(other.mtx), alloc) {}
		mathematical_dynamic_matrix(std::initializer_list<std::initializer_list<value_type>> mat_init_list, const Allocator& alloc = Allocator())
			: mtx(mat_init_list, alloc) {}
		~mathematical_dynamic_matrix() {}
		mathematical_dynamic_matrix& operator=(const mathematical_dynamic_matrix& other) {
			if (this != &other) mathematical_dynamic_matrix(other).swap(*this); // copy-swap
			return *this;
		}
		mathematical_dynamic_matrix& operator=(mathematical_dynamic_matrix&& other) {
			if (this != &other) swap(*this, other);
			return *this;
		}
		mathematical_dynamic_matrix& operator=(std::initializer_list<std::initializer_list<value_type>> ilist) {
			mtx = ilist;
			return *this;
		}
		allocator_type get_allocator() const { return mtx.get_allocator(); }
		// CAPACITY

		// ELEMENT ACCESS
		void swap(mathematical_dynamic_matrix& other) { mtx.swap(other.mtx); }
		
	private:
		matrix_type mtx;
	};
}

#endif // !MATHEMATICAL_DYNAMIC_MATRIX_H

