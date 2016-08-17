#ifndef DYNAMIC_R3_TENSOR_H
#define DYNAMIC_R3_TENSOR_H
#include "dynamic_matrix.h"
#include <algorithm>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace crsc {
	/**
	 * \class dynamic_r3_tensor
	 *
	 * \brief A container encapsulating a `std::vector` using a row-major configuration to store a rank 3 tensor-style object. The
	 *        number of elements in every row are equal, number of elements in every column are equal and number of elements in 
	 *        every slice are equal such that no holes occur in the structure.
	 *
	 * The elements of the `dynamic_r3_tensor` are stored contiguously such that they can be accessed through iterators as well as
	 * offsets on regular pointers to elements. Storage of a `dynamic_r3_tensor` is handled automatically allowing expansion and
	 * contraction when required. The total amount of allocated memory for the entire tensor can be queried via the `capacity()` 
	 * method. Extra memory currently not in use by the `dynamic_r3_tensor` instance can be released back to the system via calling
	 * `shrink_to_fit()`.
	 *
	 * Rellocations occur when additional memory is exhausted, these can be costly in terms of performance - minimum time complexity
	 * of linear in the size of the tensor. To avoid reallocations use `reserve()` if the dimensions of the matrix will be known 
	 * beforehand (i.e. before using `push` or `insert` operations).
	 *
	 * Iteration support is via a `std::bidirectional_iterator` from the `std::vector` data structure, therefore both random access
	 * iteration and forward iteration are allowed. The order of iteration uses an "in-order traversal" such that elements of the 
	 * `dynamic_r3_tensor` are iterated through by rows from left to right, top to bottom with slices traversed from front to back.
	 *
	 * \tparam _Ty The type of the elements.
	 * \tparam _Alloc An allocator that is used to acquire memory to store the elements. The type must meet the requirements of 
	 *         `Allocator` (see C++ Standard). Behaviour is undefined if `_Alloc::value_type != _Ty`.
	 * \remark As this is a dynamic data structure the dimensions of the `dynamic_r3_tensor` do NOT need to be known at compile-time,
	 *         these dimensions can be manipulated at run-time.
	 * \invariant Every row shall have an equal number of elements, every column shall have an equal number of elements and every
	 *            slices shall have an equal number of elements such that no holes occur in the structure.
	 * \author Samuel Rowlinson
	 * \date July, 2016
	 */
	template<typename _Ty,
		class _Alloc = std::allocator<_Ty>
	> class dynamic_r3_tensor {
	public:
		// PUBLIC API TYPE DEFINITIONS
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
			const_reference operator[](size_type s) const {
				return vec[(s*tensor_rows + col_index)*tensor_columns + row_index];
			}
			reference operator[](size_type s) {
				return vec[(s*tensor_rows + col_index)*tensor_columns + row_index];
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
			proxy_column_vector operator[](size_type c) const {
				return proxy_column_vector(vec, row_index, c, tensor_columns, tensor_rows);
			}
			proxy_column_vector operator[](size_type c) {
				return proxy_column_vector(vec, row_index, c, tensor_columns, tensor_rows);
			}
		private:
			std::vector<value_type, _Alloc>& vec;
			size_type row_index;
			size_type tensor_columns;
			size_type tensor_rows;
		};
	public:
		// CONSTRUCTION/ASSIGNMENT
		/**
		 * \brief Default constructor, initialises empty container (zero rows, columns and slices) using 
		 *        the default constructed allocator.
		 *
		 * \complexity Constant.
		 */
		dynamic_r3_tensor() : dynamic_r3_tensor(_Alloc()) {}
		/**
		 * \brief Initialises empty container (zero rows, columns and slices) using a specificed allocator `alloc`.
		 *
		 * \param alloc Allocator to use for all memory allocations of this container.
		 * \complexity Constant.
		 */
		explicit dynamic_r3_tensor(const _Alloc& alloc)
			: tnsr(alloc), rows_(0), cols_(0), slices_(0) {}
		/**
		 * \brief Constructs the container with `_rows*_cols` default-inserted instances of `_Ty`.
		 *
		 * \param _row Number of rows.
		 * \param _cols Number of columns.
		 * \param _slices Number of slices.
		 * \param alloc Allocator to use for all memory allocations of this container.
		 * \complexity Linear in `_rows*_cols*_slices`.
		 */
		explicit dynamic_r3_tensor(size_type _rows, size_type _cols, size_type _slices, const _Alloc& alloc = _Alloc())
			: tnsr(_rows*_cols*_slices, alloc), rows_(_rows), cols_(_cols), slices_(_slices) {}
		/**
		 * \brief Constructs the container with `_rows*_cols*_slices` copies of elements with value `_val`.
		 *
		 * \param _rows Number of rows.
		 * \param _cols Number of columns.
		 * \param _slices Number of slices.
		 * \param _val Value to initialise elements of the container with.
		 * \param alloc Allocator to use for all memory allocations of this container.
		 * \complexity Linear in `_rows*_cols*_slices`.
		 */
		explicit dynamic_r3_tensor(size_type _rows, size_type _cols, size_type _slices, const value_type& _val, const _Alloc& alloc = _Alloc())
			: tnsr(_rows*_cols*_slices, _val, alloc), rows_(_rows), cols_(_cols), slices_(_slices) {}
		/**
		 * \brief Constructs the container with the contents of a three-dimensional C-style array `arr_3d`.
		 *
		 * \param arr_3d Three-dimensional C-style array used as source to initialise elements of the container with.
		 * \param _rows Number of rows.
		 * \param _cols Number of columns.
		 * \param _slices Number of slices.
		 * \param alloc Allocator to use for all memory allocations of this container.
		 * \complexity Linear in `_rows*_cols*_slices`.
		 */
		explicit dynamic_r3_tensor(value_type*** arr_3d, size_type _rows, size_type _cols, size_type _slices, const _Alloc& alloc = _Alloc())
			: tnsr(_rows*_cols*_slices, alloc), rows_(_rows), cols_(_cols), slices_(_slices) {
			for (size_type i = 0; i < _rows; ++i) {
				for (size_type j = 0; j < _cols; ++j) {
					for (size_type k = 0; k < _slices; ++k)
						tnsr[(k*_rows + j)*_cols + i] = arr_3d[i][j][k];
				}
			}
		}
		/**
		 * \brief Copy constructor. Constructs the container with the copy of the contents of `_other`. Allocator is obtained through
		 *        calling `std::allocator_traits<allocator_type>::select_on_container_copy_construction(_other.get_allocator())`.
		 *
		 * \param _other Another `dynamic_r3_tensor` container to be used as source to initialise elements of the container with.
		 * \complexity Linear in `_other.rows()*_other.columns()*_other.slices()`.
		 */
		dynamic_r3_tensor(const dynamic_r3_tensor& _other)
			: tnsr(_other.tnsr), rows_(_other.rows_), cols_(_other.cols_), slices_(_other.slices_) {}
		/**
		 * \brief Copy constructor. Constructs the container with the copy of the contents of `_other`.
		 *
		 * \param _other Another `dynamic_r3_tensor` container to be used as source to initialise elements of the container with.
		 * \param alloc Allocator to use for all memory allocations of this container.
		 * \complexity Linear in `_other.rows()*_other.columns()`.
		 */
		dynamic_r3_tensor(const dynamic_r3_tensor& _other, const _Alloc& alloc)
			: tnsr(_other.tnsr, alloc), rows_(_other.rows_), cols_(_other.cols_), slices_(_other.slices_) {}
		/**
		 * \brief Move constructor. Constructs the container with the contents of `_other` using move-semantics. Allocator is
		 *        obtained through move-construction from the allocator belonging to `_other`.
		 *
		 * \param _other Another `dynamic_r3_tensor` container to be used as source to initialise elements of the container with.
		 * \complexity Constant.
		 */
		dynamic_r3_tensor(dynamic_r3_tensor&& _other)
			: tnsr(std::move(_other.tnsr)), rows_(std::move(_other.rows_)), cols_(std::move(_other.cols_)), slices_(std::move(_other.slices_)) {}
		/**
		 * \brief Move constructor. Constructs the container with the contents of `_other` using move-semantics.
		 *
		 * \param _other Another `dynamic_r3_tensor` container to be used as source to initialise elements of the container with.
		 * \param alloc Allocator to use for all memory allocations of this container.
		 * \complexity If `alloc != _other.get_allocator()` linear, otherwise constant.
		 */
		dynamic_r3_tensor(dynamic_r3_tensor&& _other, const _Alloc& alloc)
			: tnsr(std::move(_other.tnsr), alloc), rows_(std::move(_other.rows_)), cols_(std::move(_other.cols_)), slices_(std::move(_other.slices_)) {}
		/**
		 * \brief Destructs the container. The destructors of the elements are called and the used storage is deallocated. Note
		 *        that if the elements are raw-pointers, the pointed-to-objects are not destroyed. Use smart pointers instead
		 *        of raw pointers to ensure memory handling takes place.
		 *
		 * \complexity Linear in `rows()*columns()*slices()`.
		 */
		~dynamic_r3_tensor() {}
		/**
		 * \brief Copy-assignment operator. Replaces the contents of the container with a copy of the contents of `_other`.
		 *
		 * \param _other Another `dynamic_r3_tensor` container to use as data source.
		 * \return `*this`.
		 * \complexity Linear in the size of `*this` and `_other`.
		 */
		dynamic_r3_tensor& operator=(const dynamic_r3_tensor& _other) {
			if (this != &_other)
				dynamic_r3_tensor(_other).swap(*this);
			return *this;
		}
		/**
		 * \brief Move-assignment operator. Replaces the contents of the container with those of `_other` using move-semantics.
		 *
		 * \param _other Another `dynamic_r3_tensor` container to use as data source.
		 * \return `*this`.
		 * \complexity Linear in the size of `*this` unless the allocators do not compare equal and do not propagate, in which
		 *             case linear in the size of `*this` and `_other`.
		 */
		dynamic_r3_tensor& operator=(dynamic_r3_tensor&& _other) {
			if (this != &_other)
				swap(*this, _other);
			return *this;
		}
		/**
		 * \brief Returns the allocator associated with the container.
		 *
		 * \return The associated allocator.
		 * \complexity Constant.
		 */
		allocator_type get_allocator() const {
			return tnsr.get_allocator();
		}
		// CAPACITY
		/**
		 * \brief Checks if the container has no elements.
		 *
		 * \return `true` if the container is empty, `false` otherwise.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		bool empty() const noexcept {
			return tnsr.empty();
		}
		/**
		 * \brief Returns the number of rows in the container.
		 *
		 * \return Number of rows.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		size_type rows() const noexcept {
			return rows_;
		}
		/**
		 * \brief Returns the number of columns in the container.
		 *
		 * \return Number of columns.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		size_type columns() const noexcept {
			return cols_;
		}
		/**
		 * \brief Returns the number of slices in the container.
		 *
		 * \return Number of slices.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		size_type slices() const noexcept {
			return slices_;
		}
		/**
		 * \brief Returns the size of the container in terms of number of elements it holds, 
		 *        equal to the product `rows()*columns()*slices()`.
		 *
		 * \return Number of elements in the container.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		size_type size() const noexcept {
			return tnsr.size();
		}
		/**
		 * \brief Returns the maximum number of elements the container is able to hold due to
		 *        system or library implementation limitations.
		 *
		 * \return Maximum number of elements.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		size_type max_size() const noexcept {
			return tnsr.max_size();
		}
		/**
		 * \brief Returns the number of elements that the container has currently allocated space for.
		 *
		 * \return Size of the current allocated storage.
	 	 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		size_type capacity() const noexcept {
			return tnsr.capacity();
		}
		/**
	 	 * \brief Increases the capacity of the container to a value that's greater than or equal
		 *        to `_rows*_cols*_slices`, if `_rows*_cols*_slices` is greater than current
		 *        `capacity()` new storage is allocated, otherwise the method does nothing.
		 *
		 * \param _rows Number of rows to allocate storage for.
		 * \param _cols Number of columns to allocate storage for.
		 * \param _slices Number of slices to allocate storage for.
		 * \throw Throws `std::length_error` if `_rows*_cols*_slices > max_size()`. Or any exception
		 *        thrown by `_Alloc::allocate()` (typically `std::bad_alloc`).
		 * \exceptionsafety If no reallocations happen of if the type of the elements has either a
		 *                  non-throwing move constructor or a copy constructor, then there is a
		 *                  strong guarantee (no changes in container if exception is thrown). Otherwise
		 *                  there is a basic guarantee (container guaranteed to end in a valid state).
		 */
		void reserve(size_type _rows, size_type _cols, size_type _slices) {
			tnsr.reserve(_rows*_cols*_slices);
		}
		/**
		 * \brief Requests the removal of unused container capacity.
		 *
		 * A non-binding request to reduce `capacity()` to `size()`. It is implementation-defined therefore
		 * the request may or may not be fulfilled depending upon the implementation.
		 *
		 * \complexity At most linear in `rows()*columns()*slices()`.
		 * \exceptionsafety Technically implementation-dependent but typically if the type of the elements is
		 *                  either copyable or no-throw moveable then there is a strong guarantee (no changes
		 *                  in container if exception is thrown). Otherwise there is a basic guarantee (container
		 *                  guaranteed to end in a valid state).
		 */
		void shrink_to_fit() {
			tnsr.shrink_to_fit();
		}
		// ELEMENT ACCESS
		/**
		 * \brief Gets `const_reference` to element at specified row-column-slice indices.
		 *
		 * \param r Row position.
		 * \param c Column position.
		 * \param s Slice position.
		 * \return Constant reference to element at given position.
		 * \throw Throws `std::out_of_range` exception if any of `r`, `c` or `s` are greater
		 *        than or equal to `rows()`, `columns()`, `slices()` respectively.
		 * \complexity Constant.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no
		 *                  changes in the container.
		 */
		const_reference at(size_type r, size_type c, size_type s) const {
			if (!(r < rows_) || !(c < cols_) || !(s < slices_))
				throw std::out_of_range("dynamic_r3_tensor indices out of bounds.");
			return tnsr[(s*rows_ + c)*cols_ + r];
		}
		/**
		 * \brief Gets `reference` to element at specified row-column-slice indices.
		 *
		 * \param r Row position.
		 * \param c Column position.
		 * \param s Slice position.
		 * \return Reference to element at given position.
		 * \throw Throws `std::out_of_range` exception if any of `r`, `c` or `s` are greater
		 *        than or equal to `rows()`, `columns()`, `slices()` respectively.
		 * \complexity Constant.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no
		 *                  changes in the container.
		 */
		reference at(size_type r, size_type c, size_type s) {
			if (!(r < rows_) || !(c < cols_) || !(s < slices_))
				throw std::out_of_range("dynamic_r3_tensor indices out of bounds.");
			return tnsr[(s*rows_ + c)*cols_ + r];
		}
		/**
		 * \brief Gets a proxy object representing the row vector of the tensor at a given
		 *        row index `r` such that calling the triple subscript operator gets a 
		 *        `const_reference` to the element at the specified position.
		 *
		 * \param r Row position.
		 * \return Object of type `proxy_row_vector` corresponding to row index `r`.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `r < rows()`, otherwise undefined behaviour.
		 */
		proxy_row_vector operator[](size_type r) const {
			return proxy_row_vector(tnsr, r, cols_, rows_);
		}
		/**
		 * \brief Gets a proxy object representing the row vector of the tensor at a given
		 *        row index `r` such that calling the triple subscript operator gets a
		 *        `reference` to the element at the specified position.
		 *
		 * \param r Row position.
		 * \return Object of type `proxy_row_vector` corresponding to row index `r`.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `r < rows()`, otherwise undefined behaviour.
		 */
		proxy_row_vector operator[](size_type r) {
			return proxy_row_vector(tnsr, r, cols_, rows_);
		}
		/**
		 * \brief Gets `const_reference` to element at specified row-column-slice indices.
		 *
		 * \param r Row position.
		 * \param c Column position.
		 * \param s Slice position.
		 * \return Constant reference to element at given position.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `r < rows() && c < columns() && s < slices()`,
		 *                  otherwise undefined behaviour.
		 */
		const_reference operator()(size_type r, size_type c, size_type s) const {
			return tnsr[(s*rows_ + c)*cols_ + r];
		}
		/**
		 * \brief Gets `reference` to element at specified row-column-slice indices.
		 *
		 * \param r Row position.
		 * \param c Column position.
		 * \param s Slice position.
		 * \return Reference to element at given position.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `r < rows() && c < columns() && s < slices()`,
		 *                  otherwise undefined behaviour.
		 */
		reference operator()(size_type r, size_type c, size_type s) {
			return tnsr[(s*rows_ + c)*cols_ + r];
		}
		/**
		 * \brief Returns a const_reference to first element in the container.
		 *
		 * \return Constant reference to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `!empty()`, otherwise undefined behaviour.
		 */
		const_reference front() const {
			return tnsr.front();
		}
		/**
		 * \brief Returns a reference to first element in the container.
		 *
		 * \return Reference to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `!empty()`, otherwise undefined behaviour.
		 */
		reference front() {
			return tnsr.front();
		}
		/**
		 * \brief Returns a const_reference to last element in the container.
		 *
		 * \return Constant reference to the last element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `!empty()`, otherwise undefined behaviour.
		 */
		const_reference back() const {
			return tnsr.back();
		}
		/**
		 * \brief Returns a reference to last element in the container.
		 *
		 * \return Reference to the last element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `!empty()`, otherwise undefined behaviour.
		 */
		reference back() {
			return tnsr.back();
		}
		/**
		 * \brief Returns `const_pointer` to the underlying array `const _Ty*` serving as element
		 *        storage.
		 *
		 * This pointer is such that the range `[data(), data() + size()]` is always a valid range,
		 * even if the container is empty - `data()` is non-dereferencable in this case.
		 *
		 * \return Constant pointer to underlying element storage, for non empty container => `&front()`.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_pointer data() const noexcept {
			return tnsr.data();
		}
		/**
		 * \brief Returns `pointer` to the underlying array `_Ty*` serving as element storage.
		 *
		 * This pointer is such that the range `[data(), data() + size()]` is always a valid range,
		 * even if the container is empty - `data()` is non-dereferencable in this case.
		 *
		 * \return Pointer to underlying element storage, for non empty container = > `&front()`.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		pointer data() noexcept {
			return tnsr.data();
		}
		/**
		 * \brief Sends the container data to a `std::ostream` instance in a mathematical-tensor style format.
		 *
		 * \param _os Instance of `std::ostream` to write to.
		 * \param _delims `std::pair` of delimiters, where first is element column-separator and second is slice seperator.
		 * \return Modified reference to `_os` containing the container data.
		 * \complexity Linear in `rows()*columns()*slices()`.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<has_insertion_operator<_Uty>::value>
		> std::ostream& write(std::ostream& _os, const std::pair<char, char>& _delims = { ' ', ',' }) const {
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
		// ITERATORS
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
		// OPERATIONS/MODIFIERS
		/**
		 * \brief Removes all elements from the container but leaves the `capacity()` unchanged.
		 *
		 * \complexity Linear in `rows()*columns()*slices()`.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		void clear() {
			tnsr.clear();
			rows_ = 0U;
			cols_ = 0U;
			slices_ = 0U;
		}
		/**
		 * \brief Assigns the given value `_val` to all elements in the container.
		 *
		 * \param _val Value to assign to all elements.
		 * \complexity Exactly `rows()*columns()*slices()` assignments.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		void fill(const value_type& _val) noexcept {
			std::fill(tnsr.begin(), tnsr.end(), _val);
		}
		/**
		* \brief Exchanges the contents of the container with those of `_other`. Does not cause iterators
		*        and references to associate with the other container.
		*
		 * \param _other `dynamic_r3_tensor` container to swap with.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee.
		 */
		void swap(dynamic_r3_tensor& _other) {
			tnsr.swap(_other.tnsr);
			std::swap(rows_, _other.rows_);
			std::swap(cols_, _other.cols_);
			std::swap(slices_, _other.slices_);
		}
		/**
		 * \brief Exchanges the contents of two `crsc::dynamic_r3_tensor` containers. Does not cause
		 *        iterators and references to associate with the other containers.
		 *
		 * \param lhs First instance of `dynamic_r3_tensor`.
		 * \param rhs Second instance of `dynamic_r3_tensor`.
	 	 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee.
		 */
		static void swap(dynamic_r3_tensor& lhs, dynamic_r3_tensor& rhs) {
			lhs.swap(rhs);
		}
	private:
		std::vector<value_type, _Alloc> tnsr;
		size_type rows_;
		size_type cols_;
		size_type slices_;
	};
	/**
	 * \brief Stream insertion operator. Inserts formatted `dynamic_r3_tensor` contents to a `std::ostream`.
	 *
	 * \param os Instance of `std::ostream` to write to.
	 * \param tnsr `dynamic_r3_tensor` object to write to stream.
	 * \return Modified reference to `os` containing the container data.
	 * \complexity Linear in `rows()*columns()*slices()`.
	 * \exceptionsafety No-throw guarantee, `noexcept` specification.
	 */
	template<typename _Ty,
		class _Alloc = std::allocator<_Ty>,
		class = std::enable_if_t<has_insertion_operator<_Ty>::value>
	> std::ostream& operator<<(std::ostream& os, const dynamic_r3_tensor<_Ty, _Alloc>& tnsr) {
		return tnsr.write(os);
	}
	template<typename _Ty,
		class _Alloc = std::allocator<_Ty>
	> dynamic_r3_tensor<_Ty, _Alloc> to_dynamic_r3_tensor(_Ty*** arr_3d,
		std::size_t rows, std::size_t cols, std::size_t slices, const _Alloc& alloc = _Alloc()) {
		dynamic_r3_tensor<_Ty, _Alloc> dyntnsr(arr_3d, rows, cols, slices, alloc);
		for (std::size_t i = 0; i < rows; ++i) {
			for (std::size_t j = 0; j < cols; ++j) {
				delete[] arr_3d[i][j];
			}
			delete[] arr_3d[i];
		}
		delete[] arr_3d;
		return dyntnsr;
	}
	template<typename _Ty,
		class _Alloc = std::allocator<_Ty>
	> dynamic_r3_tensor<_Ty, _Alloc> to_dynamic_r3_tensor(_Ty*** arr_3d,
		std::size_t rows, std::size_t cols, std::size_t slices, const _Alloc& alloc = _Alloc()) {
		return dyntnsr(arr_3d, rows, cols, slices, alloc);
	}
}

#endif // !DYNAMIC_R3_TENSOR_H