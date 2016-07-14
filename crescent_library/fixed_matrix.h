#ifndef FIXED_MATRIX_H
#define FIXED_MATRIX_H
#include <algorithm>
#include <array>
#include <iterator>
#include <ostream>
#include <stdexcept>

namespace crsc {

	template<typename _Ty,
		std::size_t _Rows,
		std::size_t _Cols
	> class fixed_matrix {
	public:
		typedef _Ty value_type;
		typedef _Ty& reference;
		typedef const _Ty& const_reference;
		typedef _Ty* pointer;
		typedef const _Ty* const_pointer;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef typename std::array<_Ty, _Rows*_Cols>::const_iterator const_iterator;
		typedef typename std::array<_Ty, _Rows*_Cols>::iterator iterator;
		typedef typename std::array<_Ty, _Rows*_Cols>::const_reverse_iterator const_reverse_iterator;
		typedef typename std::array<_Ty, _Rows*_Cols>::reverse_iterator reverse_iterator;
	private:
		class proxy_row_array {
		public:
			proxy_row_array(std::array<value_type, _Rows*_Cols>& _row_array, size_type _row_index, size_type _cols)
				: row_array(_row_array), row_index(_row_index), cols(_cols) {}
			constexpr const_reference operator[](size_type _col_index) const {
				return row_array[row_index*cols + _col_index];
			}
			reference operator[](size_type _col_index) {
				return row_array[row_index*cols + _col_index];
			}
		private:
			std::array<value_type, _Rows*_Cols>& row_array;
			size_type row_index;
			size_type cols;
		};
	public:
		fixed_matrix() : mtx() {}
		fixed_matrix(const value_type& _val) : mtx() { fill(_val); }
		fixed_matrix(const value_type** _c_arr_2d) : mtx() {
			for (size_type i = 0; i < _Rows; ++i) {
				for (size_type j = 0; j < _Cols; ++j)
					mtx[i*_Cols + j] = _c_arr_2d[i][j];
			}
		}
		fixed_matrix(const fixed_matrix& _other) : mtx(_other.mtx) {}
		fixed_matrix(fixed_matrix&& _other) : mtx(std::move(_other.mtx)) {}
		~fixed_matrix() {}
		fixed_matrix& operator=(const fixed_matrix& _other) {
			if (this != &_other)
				return fixed_matrix(_other).swap(*this);
			return *this;
		}
		fixed_matrix& operator=(fixed_matrix&& _other) {
			if (this != &_other)
				return fixed_matrix(std::move(_other)).swap(*this);
			return *this;
		}

		// CAPACITY
		
		constexpr bool empty() const noexcept {
			return mtx.empty();
		}
		constexpr size_type size() const noexcept {
			return mtx.size();
		}
		constexpr size_type max_size() const noexcept {
			return mtx.max_size();
		}

		// ELEMENT ACCESS

		constexpr const_reference at(size_type _row_index, size_type _col_index) const {
			if (_row_index >= _Rows || _col_index >= _Cols)
				throw std::out_of_range("fixed_matrix index out of bounds.");
			return mtx.at(_row_index*_Cols + _col_index);
		}
		reference at(size_type _row_index, size_type _col_index) {
			if (_row_index >= _Rows || _col_index >= _Cols)
				throw std::out_of_range("fixed_matrix index out of bounds.");
			return mtx.at(_row_index*_Cols + _col_index);
		}
		constexpr proxy_row_array operator[](size_type _row_index) const {
			return proxy_row_array(mtx, _row_index, _Cols);
		}
		proxy_row_array operator[](size_type _row_index) {
			return proxy_row_array(mtx, _row_index, _Cols);
		}
		constexpr const_reference front() const {
			return mtx.front();
		}
		reference front() {
			return mtx.front();
		}
		constexpr const_reference back() const {
			return mtx.back();
		}
		reference back() {
			return mtx.back();
		}
		const_pointer data() const {
			return mtx.data();
		}
		pointer data() {
			return mtx.data();
		}

		// ITERATORS

		const_iterator cbegin() const {
			return mtx.cbegin();
		}
		iterator begin() {
			return mtx.begin();
		}
		const_iterator cend() const {
			return mtx.cend();
		}
		iterator end() {
			return mtx.end();
		}
		const_reverse_iterator crbegin() const {
			return mtx.crbegin();
		}
		reverse_iterator rbegin() {
			return mtx.rbegin();
		}
		const_reverse_iterator crend() const {
			return mtx.crend();
		}
		reverse_iterator rend() {
			return mtx.rend();
		}

		// OPERATIONS

		void fill(const value_type& _val) {
			mtx.fill(_val);
		}
		void swap(fixed_matrix& _other) {
			mtx.swap(_other.mtx);
		}
		fixed_matrix<value_type, _Rows - 1, _Cols - 1> submatrix(size_type _row_index, size_type _col_index) const noexcept {
			fixed_matrix<value_type, _Rows - 1, _Cols - 1> sub;
			size_type row_erased = 0;
			size_type col_erased = 0;
			for (size_type i = 0; i < rows_ - 1; ++i) {
				col_erased = 0;
				for (size_type j = 0; j < cols_ - 1; ++j) {
					if (i == _row_index)
						row_erased = 1;
					if (j == _col_index)
						col_erased = 1;
					sub.at(i, j) = at(i + row_erased, j + col_erased);
				}
			}
			return sub;
		}
		value_type trace() const {
			if (_Rows != _Cols)
				throw std::logic_error("cannot compute trace() of non-square matrix.");
			value_type result = value_type();
			for (size_type i = 0; i < _Rows; ++i)
				result += mtx[i*(_Cols + 1)];
			return result;
		}

		// OPERATORS

		fixed_matrix& operator+=(const fixed_matrix& _other) {
			for (size_type i = 0; i < _Rows; ++i) {
				for (size_type j = 0; j < _Cols; ++j)
					operator[](i)[j] += _other.at(i, j);
			}
			return *this;
		}
		fixed_matrix& operator-=(const fixed_matrix& _other) {
			for (size_type i = 0; i < _Rows; ++i) {
				for (size_type j = 0; j < _Cols; ++j)
					operator[](i)[j] -= _other.at(i, j);
			}
			return *this;
		}
		fixed_matrix operator+(const fixed_matrix& _other) const {
			fixed_matrix sum;
			for (size_type i = 0; i < _Rows; ++i) {
				for (size_type j = 0; j < _Cols; ++j)
					sum.at(i, j) = at(i, j) + _other.at(i, j);
			}
			return sum;
		}
		fixed_matrix operator-(const fixed_matrix& _other) const {
			fixed_matrix difference;
			for (size_type i = 0; i < _Rows; ++i) {
				for (size_type j = 0; j < _Cols; ++j)
					difference.at(i, j) = at(i, j) - _other.at(i, j);
			}
			return difference;
		}
		template<
			size_type _Second_Rows,
			size_type _Second_Cols
		> fixed_matrix<value_type, _Rows, _Second_Cols> operator*(const fixed_matrix<value_type, _Second_Rows, _Second_Cols>& _other) const {
			fixed_matrix<value_type, _Rows, _Second_Cols> product;
			for (size_type i = 0; i < _Rows; ++i) {
				for (size_type j = 0; j < _Second_Cols; ++j) {
					for (size_type k = 0; k < _Cols; ++k)
						product.at(i, j) += at(i, k) * _other.at(k, j);
				}
			}
			return product;
		}
		bool operator==(const fixed_matrix& _other) const noexcept {
			if (this != &_other) {
				for (size_type i = 0; i < _Rows; ++i) {
					for (size_type j = 0; j < _Cols; ++j)
						if (at(i, j) != _other.at(i, j)) return false;
				}
			}
			return true;
		}
		bool operator!=(const fixed_matrix& _other) const noexcept {
			return !(*this == _other);
		}

	private:
		std::array<value_type, _Rows*_Cols> mtx;
	};

	template<typename _Ty,
		std::size_t _rows,
		std::size_t _cols,
		class = std::enable_if_t<_rows == _cols
			&& std::is_arithmetic<_Ty>::value
		>
	> fixed_matrix<_Ty, _rows, _cols> make_identity_matrix() {
		fixed_matrix<_Ty, _rows, _cols> identity_matrix();
		for (std::size_t i = 0; i < _rows; ++i) {
			for (std::size_t j = 0; j < _cols; ++j)
				if (i == j) identity_matrix[i][j] = static_cast<_Ty>(1);
		}
		return identity_matrix;
	}

	template<typename _Ty,
		std::size_t _rows,
		std::size_t _cols
	> fixed_matrix<_Ty, _rows, _cols> to_fixed_matrix(const _Ty** c_arr_2d) {
		return fixed_matrix<_Ty, _rows, _cols>(c_arr_2d);
	}

}

#endif // !FIXED_MATRIX_H
