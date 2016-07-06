#ifndef DYNAMIC_R3_TENSOR_H
#define DYNAMIC_R3_TENSOR_H
#include "dynamic_matrix.h"
#include <algorithm>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace crsc {

	template<typename _Ty,
		class _Alloc = std::allocator<_Ty>
	> class dynamic_r3_tensor {
	public:
		// public API type definitions
		typedef _Ty value_type;
		typedef _Ty& reference;
		typedef const _Ty& const_reference;
		typedef _Ty* pointer;
		typedef const _Ty* const_pointer;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef _Alloc allocator_type;
		typedef typename std::vector<_Ty, _Alloc>::const_iterator const_iterator;
		typedef typename std::vector<_Ty, _Alloc>::iterator iterator;
		typedef typename std::vector<_Ty, _Alloc>::const_reverse_iterator const_reverse_iterator;
		typedef typename std::vector<_Ty, _Alloc>::reverse_iterator reverse_iterator;
	private:
		/**
		 * \class proxy_column_vector
		 *
		 * \brief Proxy class used for enabling operator[][][] overload on dynamic_r3_tensor objects, represents
		 *        a "column-vector" of the rank-3 tensor.
		 */
		class proxy_column_vector {
		public:
			proxy_column_vector(std::vector<value_type, _Alloc>& _vec, size_type _row_index, size_type _col_index, size_type _cols, size_type _rows)
				: vec(_vec), row_index(_row_index), col_index(_col_index), tensor_columns(_cols), tensor_rows(_rows) {
			}
			const_reference operator[](size_type _slice_index) const {
				return vec[(_slice_index*tensor_rows + col_index)*tensor_columns + row_index];
			}
			reference operator[](size_type _slice_index) {
				return vec[(_slice_index*tensor_rows + col_index)*tensor_columns + row_index];
			}
		private:
			std::vector<value_type, _Alloc>& vec;
			size_type row_index;
			size_type col_index;
			size_type tensor_columns;
			size_type tensor_rows;
		};
		/**
		 * \class proxy_row_vector
		 *
		 * \brief Proxy class used for enabling operator[][][] overload on dynamic_r3_tensor objects, represents
		 *        a "row-vector" of the rank-3 tensor.
		 */
		class proxy_row_vector {
		public:
			proxy_row_vector(std::vector<value_type, _Alloc>& _vec, size_type _row_index, size_type _cols, size_type _rows)
				: vec(_vec), row_index(_row_index), tensor_columns(_cols), tensor_rows(_rows) {
			}
			proxy_column_vector operator[](size_type _col_index) const {
				return proxy_column_vector(vec, row_index, _col_index, tensor_columns, tensor_rows);
			}
			proxy_column_vector operator[](size_type _col_index) {
				return proxy_column_vector(vec, row_index, _col_index, tensor_columns, tensor_rows);
			}
		private:
			std::vector<value_type, _Alloc>& vec;
			size_type row_index;
			size_type tensor_columns;
			size_type tensor_rows;
		};
	public:
		dynamic_r3_tensor() : dynamic_r3_tensor(_Alloc()) {}
		explicit dynamic_r3_tensor(const _Alloc& alloc)
			: tnsr(alloc), rows_(0), cols_(0), slices_(0) {
		}
		explicit dynamic_r3_tensor(size_type _rows, size_type _cols, size_type _slices, const _Alloc& alloc = _Alloc())
			: tnsr(_rows*_cols*_slices, alloc), rows_(_rows), cols_(_cols), slices_(_slices) {
		}
		explicit dynamic_r3_tensor(size_type _rows, size_type _cols, size_type _slices, const value_type& _val, const _Alloc& alloc = _Alloc())
			: tnsr(_rows*_cols*_slices, _val, alloc), rows_(_rows), cols_(_cols), slices_(_slices) {
		}
		dynamic_r3_tensor(const dynamic_r3_tensor& _other)
			: tnsr(_other.tnsr), rows_(_other.rows_), cols_(_other.cols_), slices_(_other.slices_) {
		}
		dynamic_r3_tensor(const dynamic_r3_tensor& _other, const _Alloc& alloc)
			: tnsr(_other.tnsr, alloc), rows_(_other.rows_), cols_(_other.cols_), slices_(_other.slices_) {
		}
		dynamic_r3_tensor(dynamic_r3_tensor&& _other)
			: tnsr(std::move(_other.tnsr)), rows_(std::move(_other.rows_)), cols_(std::move(_other.cols_)), slices_(std::move(_other.slices_)) {
		}
		dynamic_r3_tensor(dynamic_r3_tensor&& _other, const _Alloc& alloc)
			: tnsr(std::move(_other.tnsr), alloc), rows_(std::move(_other.rows_)), cols_(std::move(_other.cols_)), slices_(std::move(_other.slices_)) {
		}
		dynamic_r3_tensor& operator=(const dynamic_r3_tensor& _other) {
			if (this != &_other)
				dynamic_r3_tensor(_other).swap(*this);
			return *this;
		}
		dynamic_r3_tensor& operator=(dynamic_r3_tensor&& _other) {
			if (this != &_other)
				dynamic_r3_tensor(std::move(_other)).swap(*this);
			return *this;
		}
		allocator_type get_allocator() const {
			return tnsr.get_allocator();
		}

		// Capacity

		bool empty() const noexcept {
			return tnsr.empty();
		}
		size_type rows() const noexcept {
			return rows_;
		}
		size_type columns() const noexcept {
			return cols_;
		}
		size_type slices() const noexcept {
			return slices_;
		}
		size_type size() const noexcept {
			return tnsr.size();
		}
		size_type max_size() const noexcept {
			return tnsr.max_size();
		}
		size_type capacity() const noexcept {
			return tnsr.capacity();
		}
		void reserve(size_type _rows, size_type _cols, size_type _slices) {
			tnsr.reserve(_rows*_cols*_slices);
		}
		void shrink_to_fit() {
			tnsr.shrink_to_fit();
		}

		// Element Access

		const_reference at(size_type _row_index, size_type _col_index, size_type _slice_index) const {
			if (_row_index >= rows_ || _col_index >= cols_ || _slice_index >= slices_)
				throw std::out_of_range("tensor indices out of bounds.");
			return tnsr.at((_slice_index*rows_ + _col_index)*cols_ + _row_index);
		}
		reference at(size_type _row_index, size_type _col_index, size_type _slice_index) {
			if (_row_index >= rows_ || _col_index >= cols_ || _slice_index >= slices_)
				throw std::out_of_range("tensor indices out of bounds.");
			return tnsr.at((_slice_index*rows_ + _col_index)*cols_ + _row_index);
		}
		proxy_row_vector operator[](size_type _row_index) const {
			return proxy_row_vector(tnsr, _row_index, cols_, rows_);
		}
		proxy_row_vector operator[](size_type _row_index) {
			return proxy_row_vector(tnsr, _row_index, cols_, rows_);
		}
		const_reference front() const {
			return tnsr.front();
		}
		reference front() {
			return tnsr.front();
		}
		const_reference back() const {
			return tnsr.back();
		}
		reference back() {
			return tnsr.back();
		}
		const_pointer data() const noexcept {
			return tnsr.data();
		}
		pointer data() noexcept {
			return tnsr.data();
		}
		std::ostream& write(std::ostream& _os, const std::pair<char, char>& _delims = { ' ', ',' }) const {
			for (size_type i = 0; i < rows_; ++i) {
				for (size_type j = 0; j < cols_; ++j) {
					_os << "(";
					for (size_type k = 0; k < slices_; ++k) {
						if (k != slices_ - 1) _os << at(i, j, k) << _delims.second;
						else _os << at(i, j, k);
					}
					_os << ')' << _delims.first;
				}
				_os << '\n';
			}
			return _os;
		}

		// Iterators

		const_iterator cbegin() const {
			return tnsr.cbegin();
		}
		iterator begin() {
			return tnsr.begin();
		}
		const_iterator cend() const {
			return tnsr.cend();
		}
		iterator end() {
			return tnsr.end();
		}
		const_reverse_iterator crbegin() const {
			return tnsr.crbegin();
		}
		reverse_iterator rbegin() {
			return tnsr.rbegin();
		}
		const_reverse_iterator crend() const {
			return tnsr.crend();
		}
		reverse_iterator rend() {
			return tnsr.end();
		}

		// Operations/Modifiers

		void clear() {
			tnsr.clear();
			rows_ = static_cast<size_type>(0);
			cols_ = static_cast<size_type>(0);
			slices_ = static_cast<size_type>(0);
		}
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_default_constructible<_Uty>::value>
		> iterator insert_row(size_type _row_pos, const value_type& _val = value_type()) {
			crsc::dynamic_matrix<value_type> plane_mtx(slices_, cols_, _val);
			return insert_row(_row_pos, plane_mtx);
		}
		iterator insert_row(size_type _row_pos, const crsc::dynamic_matrix<value_type>& _plane_mtx) {
			if (_row_pos > rows_)
				throw std::invalid_argument("_row_pos must be <= current value of rows().");
			if (_plane_mtx.columns() > cols_ || _plane_mtx.rows() > slices_)
				throw std::invalid_argument("_plane_mtx.columns() must be <= columns() and _plane_mtx.rows() must be <= slices().");
			++rows_;
			if (_plane_mtx.columns() == cols_ && _plane_mtx.rows() == slices_)
				return tnsr.insert(tnsr.cbegin() + _row_pos*cols_, _plane_mtx.cbegin(), _plane_mtx.cend());
			crsc::dynamic_matrix<value_type> resized = _plane_mtx;
			resized.resize(slices_, cols_);
			return tnsr.insert(tnsr.cbegin() + _row_pos*cols_, resized.cbegin(), resized.cend());
		}
		void swap(dynamic_r3_tensor& _other) {
			tnsr.swap(_other.tnsr);
			size_type tmp_rows = rows_;
			size_type tmp_cols = cols_;
			size_type tmp_slices = slices_;
			rows_ = _other.rows_;
			cols_ = _other.cols_;
			slices_ = _other.slices_;
			_other.rows_ = tmp_rows;
			_other.cols_ = tmp_cols;
			_other.slices_ = tmp_slices;
		}

	private:
		std::vector<value_type, _Alloc> tnsr;
		size_type rows_;
		size_type cols_;
		size_type slices_;
	};

}

#endif /* DYNAMIC_R3_TENSOR_H */