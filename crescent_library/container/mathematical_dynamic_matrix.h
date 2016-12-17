#ifndef MATHEMATICAL_DYNAMIC_MATRIX_H
#define MATHEMATICAL_DYNAMIC_MATRIX_H
#include "dynamic_matrix.h"

namespace crsc {
	template<typename Ty,
		class Allocator = std::allocator<Ty>
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
			if (this != &other) mtx = std::move(other.mtx);
			return *this;
		}
		mathematical_dynamic_matrix& operator=(std::initializer_list<std::initializer_list<value_type>> ilist) {
			mtx = ilist;
			return *this;
		}
		allocator_type get_allocator() const { return mtx.get_allocator(); }
		// CAPACITY
		bool empty() const noexcept { return mtx.empty(); }
		size_type rows() const noexcept { return mtx.rows(); }
		size_type columns() const noexcept { return mtx.columns(); }
		size_type size() const noexcept { return mtx.size(); }
		size_type max_size() const noexcept { return mtx.max_size(); }
		size_type capacity() const noexcept { return mtx.capacity(); }
		void reserve(size_type rows, size_type columns) { mtx.reserve(rows, columns); }
		void shrink_to_fit() { mtx.shrink_to_fit(); }
		// ELEMENT ACCESS
		const_reference at(size_type i, size_type j) const { return mtx.at(i, j); }
		reference at(size_type i, size_type j) { return mtx.at(i, j); }
		// TODO: operator[][] 
		const_reference operator()(size_type i, size_type j) const { return mtx(i, j); }
		reference operator()(size_type i, size_type j) { return mtx(i, j); }
		const_reference front() const { return mtx.front(); }
		reference front() { return mtx.front(); }
		const_reference back() const { return mtx.back(); }
		reference back() { return mtx.back(); }
		const_pointer data() const noexcept { return mtx.data(); }
		pointer data() noexcept { return mtx.data(); }
		// OPERATIONS
		void clear() noexcept { mtx.clear(); }
		iterator insert_row(size_type row_pos, const value_type& val) {
			return mtx.insert_row(row_pos, val);
		}
		iterator insert_row(size_type row_pos, const std::vector<value_type>& row_vec) {
			return mtx.insert_row(row_pos, row_vec);
		}
		iterator insert_row(size_type row_pos, std::vector<value_type>&& row_vec = std::vector<value_type>()) {
			return mtx.insert_row(row_pos, std::move(row_vec));
		}
		iterator insert_column(size_type col_pos, const value_type& val) {
			return mtx.insert_column(col_pos, val);
		}
		iterator insert_column(size_type col_pos, const std::vector<value_type>& col_vec) {
			return mtx.insert_column(col_pos, col_vec);
		}
		iterator insert_column(size_type col_pos, std::vector<value_type>&& col_vec = std::vector<value_type>()) {
			return mtx.insert_column(col_pos, std::move(col_vec));
		}
		iterator erase_row(size_type row_pos) { return mtx.erase_row(row_pos); }
		iterator erase_column(size_type col_pos) { return mtx.erase_column(col_pos); }
		void fill(const value_type& val) noexcept { mtx.fill(val); }
		void push_row(const value_type& val) { mtx.push_row(val); }
		void push_row(const std::vector<value_type>& row_vec) { mtx.push_row(row_vec); }
		void push_row(std::vector<value_type>&& row_vec = std::vector<value_type>()) { mtx.push_row(std::move(row_vec)); }
		void push_column(const value_type& val) noexcept { mtx.push_column(val); }
		void push_column(const std::vector<value_type>& col_vec) { mtx.push_column(col_vec); }
		void push_column(std::vector<value_type>&& col_vec = std::vector<value_type>()) { mtx.push_column(std::move(col_vec)); }
		void pop_row() { mtx.pop_row(); }
		void pop_column() { mtx.pop_column(); }
		void rows_resize(size_type rows) { mtx.rows_resize(rows); }
		void rows_resize(size_type rows, const value_type& val) { mtx.rows_resize(rows, val); }
		void columns_resize(size_type columns) { mtx.columns_resize(columns); }
		void columns_resize(size_type columns, const value_type& val) { mtx.columns_resize(columns, val); }
		void resize(size_type rows, size_type columns) { mtx.resize(rows, columns); }
		void resize(size_type rows, size_type columns, const value_type& val) { mtx.resize(rows, columns, val); }
		void swap(mathematical_dynamic_matrix& other) { mtx.swap(other.mtx); }
		mathematical_dynamic_matrix& submatrix(size_type i, size_type j) {
			mtx.submatrix(i, j);
			return *this;
		}
		value_type trace() const { return matrix_trace(mtx); }
		// OPERATORS
		bool operator==(const mathematical_dynamic_matrix& other) const noexcept { return mtx == other.mtx; }
		bool operator!=(const mathematical_dynamic_matrix& other) const noexcept { return !(*this == other); }
		mathematical_dynamic_matrix& operator+=(const mathematical_dynamic_matrix& other) {
			for (size_type i = 0; i < rows(); ++i) {
				for (size_type j = 0; j < columns(); ++j)
					mtx(i, j) += other.mtx(i, j);
			}
			return *this;
		}
		mathematical_dynamic_matrix& operator-=(const mathematical_dynamic_matrix& other) {
			for (size_type i = 0; i < rows(); ++i) {
				for (size_type j = 0; j < columns(); ++j)
					mtx(i, j) -= other.mtx(i, j);
			}
			return *this;
		}
		mathematical_dynamic_matrix operator+(const mathematical_dynamic_matrix& other) {
			return matrix_sum(mtx, other.mtx);
		}
		mathematical_dynamic_matrix operator-(const mathematical_dynamic_matrix& other) {
			return matrix_difference(mtx, other.mtx);
		}
		mathematical_dynamic_matrix operator*(const mathematical_dynamic_matrix& other) {
			return matrix_product(mtx, other.mtx);
		}
		mathematical_dynamic_matrix& operator*=(const value_type& scale) {
			std::for_each(begin(), end(), [scale](auto& el) { el *= scale; });
			return *this;
		}
		mathematical_dynamic_matrix operator*(const value_type& scale) {
			mathematical_dynamic_matrix scaled_matrix(*this);
			return scaled_matrix *= scale;
		}
		// ITERATORS
		/**
		 * \brief Returns a const_iterator the first element of the container.
		 *
		 * \remark If the container is empty, the return value will be equal to `cend()`.
		 * \return Constant iterator to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_iterator cbegin() const noexcept { return mtx.cbegin(); }
		/**
		 * \brief Returns a const_iterator the first element of the container.
		 *
		 * \remark If the container is empty, the return value will be equal to `cend()`.
		 * \return Constant iterator to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_iterator begin() const noexcept { return mtx.begin(); }
		/**
		 * \brief Returns an iterator to the first element of the container.
		 *
		 * \remark If the container is empty, the return value will be equal to `end()`.
		 * \return Iterator to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		iterator begin() noexcept { return mtx.begin(); }
		/**
		 * \brief Returns a const_iterator to the past-the-end element of the container.
		 *
		 * \return Constant iterator to the past-the-end element.
		 * \complexity Constant.
	 	 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_iterator cend() const noexcept { return mtx.cend(); }
		/**
		 * \brief Returns a const_iterator to the past-the-end element of the container.
		 *
		 * \return Constant iterator to the past-the-end element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_iterator end() const noexcept { return mtx.end(); }
		/**
		 * \brief Returns an iterator to the past-the-end element of the container.
		 *
		 * \return Iterator to the past-the-end element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		iterator end() noexcept { return mtx.end(); }
		/**
		 * \brief Returns a const_reverse_iterator to the first element of the reversed container. It
		 *        corresponds to the last element of the non-reversed container.
		 *
		 * \return Constant reverse iterator to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_reverse_iterator crbegin() const noexcept { return mtx.crbegin(); }
		/**
		 * \brief Returns a const_reverse_iterator to the first element of the reversed container. It
		 *        corresponds to the last element of the non-reversed container.
		 *
		 * \return Constant reverse iterator to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_reverse_iterator rbegin() const noexcept { return mtx.rbegin(); }
		/**
		 * \brief Returns a reverse_iterator to the first element of the reversed container. It
		 *        corresponds to the last element of the non-reversed container.
		 *
		 * \return Reverse iterator to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		reverse_iterator rbegin() noexcept { return mtx.rbegin(); }
		/**
		 * \brief Returns a const_reverse_iterator to the past-the-end element of the reversed container. It
		 *        corresponds to the element preceding the first element of the non-reversed container.
		 *
		 * \return Constance reverse iterator to the past-the-end element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_reverse_iterator crend() const noexcept { return mtx.crend(); }
		/**
		 * \brief Returns a const_reverse_iterator to the past-the-end element of the reversed container. It
		 *        corresponds to the element preceding the first element of the non-reversed container.
		 *
		 * \return Constance reverse iterator to the past-the-end element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_reverse_iterator rend() const noexcept { return mtx.rend(); }
		/**
		 * \brief Returns a reverse_iterator to the past-the-end element of the reversed container. It
		 *        corresponds to the element preceding the first element of the non-reversed container.
		 *
		 * \return Reverse iterator to the past-the-end element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		reverse_iterator rend() noexcept { return mtx.rend(); }
	private:
		matrix_type mtx;
	};
	template<typename Ty,
		class Alloc = std::allocator<Ty>
	> void swap(mathematical_dynamic_matrix<Ty, Alloc>& lhs, mathematical_dynamic_matrix<Ty, Alloc>& rhs) {
		lhs.swap(rhs);
	}
	template<typename Ty,
		class Alloc = std::allocator<Ty>,
		class = std::enable_if_t<has_insertion_operator<Ty>::value>
	> std::ostream& operator<<(std::ostream& os, const mathematical_dynamic_matrix<Ty, Alloc>& mdm) {
		typename mathematical_dynamic_matrix<Ty, Alloc>::size_type count = 0U;
		for (const auto& el : mdm) {
			os << el << ' ';
			++count;
			if (!(count % mdm.columns())) os << '\n';
		}
		return os;
	}
	template<typename Ty,
		class Alloc = std::allocator<Ty>,
		class = std::enable_if_t<std::is_arithmetic<Ty>::value>
	> mathematical_dynamic_matrix<Ty, Alloc> make_identity_matrix(std::size_t rows, std::size_t columns, const Alloc& alloc = Alloc()) {
		if (rows != columns) throw std::logic_error("identity_matrix must have rows == columns.");
		mathematical_dynamic_matrix<Ty, Alloc> identity_matrix(rows, columns, alloc);
		for (typename mathematical_dynamic_matrix<Ty, Alloc>::size_type i = 0U; i < rows; ++i) {
			for (typename mathematical_dynamic_matrix<Ty, Alloc>::size_type j = 0U; j < columns; ++j)
				if (i == j) identity_matrix[i][j] = static_cast<Ty>(1);
		}
		return identity_matrix;
	}
	template<typename Ty,
		class Alloc = std::allocator<Ty>
	> mathematical_dynamic_matrix<Ty, Alloc> to_mathematical_dynamic_matrix(Ty** arr_2d, std::size_t rows, std::size_t columns, const Alloc& alloc = Alloc()) {
		mathematical_dynamic_matrix<Ty, Alloc> mdm(arr_2d, rows, columns, alloc);
		for (std::size_t i = 0; i < rows; ++i) delete[] arr_2d[i];
		delete[] arr_2d;
		return mdm;
	}
}

#endif // !MATHEMATICAL_DYNAMIC_MATRIX_H

