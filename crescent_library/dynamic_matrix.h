#ifndef DYNAMIC_MATRIX_H
#define DYNAMIC_MATRIX_H
#include "sfinae_operators.h"
#include <algorithm>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace crsc {
	/**
	 * \class dynamic_matrix
	 *
	 * \brief A container encapsulating a `std::vector` using a row-major configuration to store a matrix-style object. The
	 *        number of elements in every row are equal and the number of elements in every column are equal, such that no
	 *        holes in the structure occur.
	 *
	 * The elements of the `dynamic_matrix` are stored contiguously such that they can be accessed through iterators as well
	 * as offsets on regular pointers to elements. Storage of a `dynamic_matrix` is handled automatically allowing expansion
	 * and contraction when required. The total amount of allocated memory for the entire matrix can be queried via the
	 * `capacity()` method. Extra memory currently not in use by a `dynamic_matrix` instance can be released back to the system
	 * via calling `shrink_to_fit()`.
	 *
	 * Reallocations occur when additional memory is exhausted, these can be costly in terms of performance - minimum time
	 * complexity of linear in the size of the matrix. To avoid reallocations use `reserve()` if the dimensions of the matrix
	 * will be known beforehand (i.e. before using `push` or `insert` operations).
	 *
	 * Iteration support is via a `std::bidirectional_iterator` from the `std::vector` data structure, therefore both random
	 * access iteration and forward iteration is supported. The order of iteration uses an "in-order traversal" such that elements
	 * of the `dynamic_matrix` are iterated through by rows from left to right, top to bottom.
	 *
	 * The complexity of common operations on `dynamic_matrix` objects are:
	 *
	 * - Random access - constant O(1).
	 * - Insertion or removal of rows/columns at the end - linear in number of columns/rows respectively: \[O(n_c)\], \[O(n_r)\]. 
	 * - Insertion or removal of rows/columns - linear in `std::distance` to the end of the `dynamic_matrix` O(n).
	 *
	 * \tparam _Ty The type of the elements.
	 * \tparam _Alloc An allocator that is used to acquire memory to store the elements. The type must meet the requirements
	 *                of `Allocator` (see C++ Standard). Behaviour is undefined if `_Alloc::value_type != _Ty`.
	 * \remark As this is a dynamic data structure the dimensions of the `dynamic_matrix` do NOT need to be known at
	 *         compile-time, these dimensions can be manipulated at run-time. This structure occupies slightly more
	 *         memory than a fixed-size matrix for this reason - if memory is a concern and fixed dimensions are known
	 *         at compile-time then consider using crsc::fixed_matrix instead.
	 * \invariant Every row shall have an equal number of elements and every column shall have an equal number of elements
	 *            such that no holes occur in the structure.
	 * \author Samuel Rowlinson
	 * \date July, 2016
	 */
	template<typename _Ty,
		class _Alloc = std::allocator<_Ty>
	> class dynamic_matrix {
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
		 * \class proxy_row_vector
		 *
		 * \brief Proxy class used for enabling operator[][] overload on dynamic_matrix objects.
		 */
		class proxy_row_vector {
		public:
			proxy_row_vector(std::vector<value_type, allocator_type>& _vec, size_type _row_index, size_type _cols)
				: vec(_vec), row_index(_row_index), columns(_cols) {}
			const_reference operator[](size_type _col_index) const {
				return vec[row_index*columns + _col_index];
			}
			reference operator[](size_type _col_index) {
				return vec[row_index*columns + _col_index];
			}
		private:
			std::vector<value_type, allocator_type>& vec;
			size_type row_index;
			size_type columns;
		};
	public:
		// CONSTRUCTION/ASSIGNMENT
		/**
		 * \brief Default constructor, initialises emtpy container (zero rows and columns) using
		 *        default constructed allocator.
		 *
		 * \complexity Constant.
	 	 */
		dynamic_matrix() : dynamic_matrix(_Alloc()) {}
		/**
		 * \brief Default constructor, initialises empty container (zero rows and columns) with
		 *        specified allocator.
		 *
		 * \param alloc Allocator to use for all memory allocations of this container.
		 * \complexity Constant.
		 */
		explicit dynamic_matrix(const _Alloc& alloc)
			: mtx(alloc), rows_(0), cols_(0) {}
		/**
		 * \brief Constructs the container with `_rows*_cols` default-inserted
		 *        instances of `_Ty`.
		 *
		 * \param _rows Number of rows.
		 * \param _cols Number of columns.
		 * \param alloc Allocator to use for all memory allocations of this container.
		 * \complexity Linear in `_rows*_cols`.
		 */
		explicit dynamic_matrix(size_type _rows, size_type _cols, const _Alloc& alloc = _Alloc())
			: mtx(_rows*_cols, alloc), rows_(_rows), cols_(_cols) {}
		/**
		 * \brief Constructs the container with `_rows*_cols` copies of elements
		 *        with value `_val`.
		 *
		 * \param _rows Number of rows.
		 * \param _cols Number of columns.
		 * \param _val Value to initialise elements of the container with.
		 * \param alloc Allocator to use for all memory allocations of this container.
		 * \complexity Linear in `_rows*_cols`.
		 */
		explicit dynamic_matrix(size_type _rows, size_type _cols, const value_type& _val, const _Alloc& alloc = _Alloc())
			: mtx(_rows*_cols, _val, alloc), rows_(_rows), cols_(_cols) {}
		/**
		 * \brief Constructs the container with `_rows*cols` elements with values 
		 *        given by a 2D C-style array `_arr_2d`.
		 *
		 * \warning Undefined behaviour invoked if number of rows, columns of `_arr_2d`
		 *          not equal to `_rows`, `_cols` respectively. Additionally, `_arr_2d`
		 *          is not deleted after use, memory management of `_arr_2d` is a
		 *          responsibility of the caller.
		 * \param _arr_2d Two-dimensional C-style array used as source to 
		 *                initialise elements of the container with.
		 * \param _rows Number of rows.
		 * \param _cols Number of columns.
		 * \param alloc Allocator to use for all memory allocations of this container.
		 * \complexity Linear in `_rows*_cols`.
		 */
		explicit dynamic_matrix(value_type** _arr_2d, size_type _rows, size_type _cols, const _Alloc& alloc = _Alloc())
			: mtx(_rows*_cols, value_type(), alloc), rows_(_rows), cols_(_cols) {
			for (size_type i = 0; i < _rows; ++i) {
				for (size_type j = 0; j < _cols; ++j)
					mtx[i*_cols + j] = _arr_2d[i][j];
			}
		}
		/**
		 * \brief Copy constructor. Constructs the container with the copy of the
		 *        contents of `_other`. Allocator is obtained through calling
		 *        `std::allocator_traits<allocator_type>::select_on_container_copy_construction(_other.get_allocator())`.
		 *
		 * \param _other Another `dynamic_matrix` container to be used as source to
		 *               initialise elements of the container with.
		 * \complexity Linear in `_other.rows()*_other.columns()`.
		 */
		dynamic_matrix(const dynamic_matrix& _other)
			: mtx(_other.mtx), rows_(_other.rows_), cols_(_other.cols_) {}
		/**
		 * \brief Copy constructor. Constructs the container with the copy of the
		 *        contents of `_other`.
		 *
		 * \param _other Another `dynamic_matrix` container to be used as source to
		 *               initialise elements of the container with.
		 * \param alloc Allocator to use for all memory allocations of this container.
		 * \complexity Linear in `_other.rows()*_other.columns()`.
		 */
		dynamic_matrix(const dynamic_matrix& _other, const _Alloc& alloc)
			: mtx(_other.mtx, alloc), rows_(_other.rows_), cols_(_other.cols_) {}
		/**
		 * \brief Move constructor. Constructs the containers with the contents
		 *        of `_other` using move-semantics. Allocator is obtained by
		 *        move-construction from the allocator belonging to `_other`.
		 *
		 * \param _other Another `dynamic_matrix` container to be used as source to
		 *               initialise elements of the container with.
		 * \complexity Constant.
		 */
		dynamic_matrix(dynamic_matrix&& _other)
			: mtx(std::move(_other.mtx)), rows_(std::move(_other.rows_)), cols_(std::move(_other.cols_)) {}
		/**
		 * \brief Move constructor. Constructs the containers with the contents
		 *        of `_other` using move-semantics.
		 *
		 * \param _other Another `dynamic_matrix` container to be used as source to
		 *               initialise elements of the container with.
		 * \param alloc Allocator to use for all memory allocations of this container.
		 * \complexity If `alloc != _other.get_allocator()` linear, otherwise constant.
		 */
		dynamic_matrix(dynamic_matrix&& _other, const _Alloc& alloc)
			: mtx(std::move(_other.mtx), alloc), rows_(std::move(_other.rows_)), cols_(std::move(_other.cols_)) {}
		/**
		 * \brief Constructs the container with the contents of the nested `std::initializer_list _init_list`.
		 *
		 * \warning Undefined behaviour if sizes of each `std::initializer_list` within `_init_list` are not equivalent.
		 * \param _init_list Initializer list of initializer lists representing a matrix.
		 * \param alloc Allocator to use for all memory allocations of this container.
		 * \complexity Linear in size of `_init_list` multiplied by linear in size of each inner list.
		 */
		dynamic_matrix(std::initializer_list<std::initializer_list<value_type>> _init_list, const _Alloc& alloc = _Alloc())
			: mtx(alloc), rows_(_init_list.size()), cols_(_init_list.begin()->size()) {
			for (auto& el : _init_list) std::move(el.begin(), el.end(), std::back_inserter(mtx));
		}
		/**
		 * \brief Destructs the container. The destructors of the elements are called and the used storage is deallocated.
		 *        Note that if the elements are raw pointers, the pointed-to objects are not destroyed. Use smart pointers
		 *        instead of raw pointers to ensure memory handling takes place.
		 *
		 * \complexity Linear in `rows()*columns()`.
		 */
		~dynamic_matrix() {}
		/**
		 * \brief Copy-assignment operator. Replaces the contents of the container with a
		 *        copy of the contents of `_other`.
		 *
		 * \param _other Another `dynamic_matrix` container to use as data source.
		 * \return `*this`.
		 * \complexity Linear in the size of `*this` and `_other`.
		 */
		dynamic_matrix& operator=(const dynamic_matrix& _other) {
			if (this != &_other)
				dynamic_matrix(_other).swap(*this);	// copy-swap idiom
			return *this;
		}
		/**
		 * \brief Move-assignment operator. Replaces the contents of the container with
		 *        those of `_other` using move-semantics.
		 *
		 * \param _other Another `dynamic_matrix` container to use as data source.
		 * \return `*this`.
		 * \complexity Linear in the size of `*this` unless the allocators do not compare equal and do not propagate,
		 *             in which case linear in the size of `*this` and `_other`.
		 */
		dynamic_matrix& operator=(dynamic_matrix&& _other) {
			if (this != &_other)
				swap(*this, _other);
			return *this;
		}
		/**
		 * \brief Replaces the contents with those identified by the nested initializer list `ilist`.
		 *
		 * \param ilist Initializer list to use as data source.
		 * \return `*this`.
		 * \complexity If `ilist` dimensions equal the dimensions of the container then linear in the size of `ilist`
		 *             otherwise linear in the size of `ilist` plus a call to `resize` using `ilist` dimensions.
		 */
		dynamic_matrix& operator=(std::initializer_list<std::initializer_list<value_type>> ilist) {
			resize(ilist.size(), ilist.begin()->size());	// resize to ilist dimensions
			auto it = mtx.begin();
			for (auto& el : ilist) {	// move each inner list contents to rows of mtx
				std::move(el.begin(), el.end(), it);
				std::advance(it, cols_);
			}
			return *this;
		}
		/**
		 * \brief Returns the allocator associated with the container.
		 *
		 * \return The associated allocator.
		 * \complexity Constant.
		 */
		allocator_type get_allocator() const {
			return mtx.get_allocator();
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
			return mtx.empty();
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
		 * \brief Returns the size of the container in terms of number of elements it holds.
		 *
		 * \return Number of elements in the container.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		size_type size() const noexcept {
			return mtx.size();
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
			return mtx.max_size();
		}
		/**
		 * \brief Returns the number of elements that the container has currently allocated space for.
		 *
		 * \return Size of the current allocated storage.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		size_type capacity() const noexcept {
			return mtx.capacity();
		}
		/**
		 * \brief Increases the capacity of the container to a value that's greater than or
		 *        equal to `_rows*_cols`, if `_rows*_cols` is greater than current `capacity()`
		 *        new storage is allocated, otherwise the method does nothing.
		 *
		 * \param _rows Number of rows to allocate storage for.
		 * \param _cols Number of columns to allocate storage for.
		 * \throw Throws `std::length_error` if `_rows*_cols > max_size()`. Or any exception
		 *        thrown by `_Alloc::allocate()` (typically `std::bad_alloc`).
		 * \exceptionsafety If no reallocations happen of if the type of the elements has either a 
		 *                  non-throwing move constructor or a copy constructor, then there is a 
		 *                  strong guarantee (no changes in container if exception is thrown). Otherwise
		 *                  there is a basic guarantee (container guaranteed to end in a valid state).
		 */
		void reserve(size_type _rows, size_type _cols) {
			mtx.reserve(_rows*_cols);
		}
		/**
		 * \brief Requests the removal of unused container capacity.
		 *
		 * A non-binding request to reduce `capacity()` to `size()`. It is implementation-defined therefore
		 * the request may or may not be fulfilled depending upon the implementation.
		 *
		 * \complexity At most linear in `rows()*columns()`.
		 * \exceptionsafety Technically implementation-dependent but typically if the type of the elements is
		 *                  either copyable or no-throw moveable then there is a strong guarantee (no changes
		 *                  in container if exception is thrown). Otherwise there is a basic guarantee (container
		 *                  guaranteed to end in a valid state).
		 */
		void shrink_to_fit() {
			mtx.shrink_to_fit();
		}
		// ELEMENT ACCESS
		/**
		 * \brief Gets const_reference to element at specified row-column indices.
		 *
		 * \param _row_index Row position.
		 * \param _col_index Column position.
		 * \return Constant reference to element at given position.
		 * \throw Throws std::out_of_range exception if either of `_row_index`, `_col_index`
		 *        are greater than or equal to rows(), columns() respectively.
		 * \complexity Constant.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no
		 *                  changes in the container.
		 */
		const_reference at(size_type _row_index, size_type _col_index) const {
			if (_row_index >= rows_ || _col_index >= cols_)
				throw std::out_of_range("dynamic_matrix indices out of bounds.");
			return mtx[_row_index*cols_ + _col_index];
		}
		/**
		 * \brief Gets reference to element at specified row-column indices.
		 *
		 * \param _row_index Row position.
		 * \param _col_index Column position.
		 * \return Constant reference to element at given position.
		 * \throw Throws std::out_of_range exception if either of `_row_index`, `_col_index`
		 *        are greater than or equal to rows(), columns() respectively.
		 * \complexity Constant.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no
		 *                  changes in the container.
		 */
		reference at(size_type _row_index, size_type _col_index) {
			if (_row_index >= rows_ || _col_index >= cols_)
				throw std::out_of_range("dynamic_matrix indices out of bounds.");
			return mtx[_row_index*cols_ + _col_index];
		}
		/**
		 * \brief Gets a proxy object representing the row vector of the matrix at a given `_row_index`
		 *        such that calling the double subscript operator gets a const_reference to the element
		 *		  at the specified position.
		 *
		 * \param _row_index Row position.
		 * \return Object of type `proxy_row_vector` corresponding to given `_row_index`.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `_row_index < rows()`, otherwise undefined behaviour.
		 */
		proxy_row_vector operator[](size_type _row_index) const {
			return proxy_row_vector(mtx, _row_index, cols_);
		}
		/**
		 * \brief Gets a proxy object representing the row vector of the matrix at a given `_row_index`
		 *        such that calling the double subscript operator gets a reference to the element
		 *        at the specified position.
		 *
		 * \param _row_index Row position.
		 * \return Object of type proxy_inner_vector corresponding to given `_row_index`.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `_row_index < rows()`, otherwise undefined behaviour.
		 */
		proxy_row_vector operator[](size_type _row_index) {
			return proxy_row_vector(mtx, _row_index, cols_);
		}
		/** 
		 * \brief Gets `const_reference` to element at specified row-column indices.
		 *
		 * \param _row_index Row position.
		 * \param _col_index Column position.
		 * \return Constant reference to element at given position.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `_row_index < rows() && _col_index < columns()`, 
		 *                  otherwise undefined behaviour.
		 */
		const_reference operator()(size_type _row_index, size_type _col_index) const {
			return mtx[_row_index*cols_ + _col_index];
		}
		/**
		 * \brief Gets `reference` to element at specified row-column indices.
		 *
		 * \param _row_index Row position.
		 * \param _col_index Column position.
		 * \return Reference to element at given position.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `_row_index < rows() && _col_index < columns()`,
		 *                  otherwise undefined behaviour.
		 */
		reference operator()(size_type _row_index, size_type _col_index) {
			return mtx[_row_index*cols_ + _col_index];
		}
		/**
		 * \brief Returns a const_reference to first element in the container.
		 *
		 * \return Constant reference to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `!empty()`, otherwise undefined behaviour.
		 */
		const_reference front() const {
			return mtx.front();
		}
		/**
		 * \brief Returns a reference to first element in the container.
		 *
		 * \return Reference to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `!empty()`, otherwise undefined behaviour.
		 */
		reference front() {
			return mtx.front();
		}
		/**
		 * \brief Returns a const_reference to last element in the container.
		 *
		 * \return Constant reference to the last element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `!empty()`, otherwise undefined behaviour.
		 */
		const_reference back() const {
			return mtx.back();
		}
		/**
		 * \brief Returns a reference to last element in the container.
		 *
		 * \return Reference to the last element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `!empty()`, otherwise undefined behaviour.
		 */
		reference back() {
			return mtx.back();
		}
		/**
		 * \brief Returns const_pointer to the underlying array `const _Ty*` serving as element
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
			return mtx.data();
		}
		/**
		 * \brief Returns pointer to the underlying array `_Ty*` serving as element storage.
		 *
		 * This pointer is such that the range `[data(), data() + size()]` is always a valid range,
		 * even if the container is empty - `data()` is non-dereferencable in this case.
		 *
		 * \return Pointer to underlying element storage, for non empty container = > `&front()`.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		pointer data() noexcept {
			return mtx.data();
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
		// OPERATIONS/MODIFIERS
		/**
		 * \brief Removes all elements from the container but leaves the `capacity()` unchanged.
		 *
		 * \complexity Linear in `rows()*columns()`.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		void clear() noexcept {
			mtx.clear();
			rows_ = 0U;
			cols_ = 0U;
		}
		/**
		 * \brief Inserts a row vector to the position one slot before `_row_pos` where each
		 *        element in the inserted row will have the specified value `_val`.
		 *
		 * \param _row_pos Position one slot after insertion point.
	 	 * \param _val Value to initialise all elements of the newly inserted row with.
		 * \return Iterator pointing to the first element inserted.
		 * \throw Throws `std::invalid_argument` exception if `_row_pos > rows()`.
		 * \complexity Linear in `columns()` plus linear in distance between `_row_pos` and `end` of the container.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no changes in the container.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> iterator insert_row(size_type _row_pos, const value_type& _val) {
			return insert_row(_row_pos, std::move(std::vector<value_type>(cols_, _val)));
		}
		/**
		 * \brief Inserts a row vector to the position one slot before `_row_pos`.
		 *
		 * The row-vector inserted must have `size() == columns()` otherwise an exception is thrown. Use 
		 * `_row_vec.resize(columns())` before insertion if `_row_vec.size() < columns()` or resize this
		 * container via `columns_resize(_row_vec.size())` to accomodate the extra columns required by the
		 * row-vector to be inserted.
		 *
		 * \param _row_pos Position one slot after insertion point.
		 * \param _row_vec Instance of `std::vector` row to insert.
		 * \return Iterator pointing to the first element inserted.
		 * \throw Throws `std::out_of_range` exception, `std::invalid_argument` exception 
		 *        if `_row_pos > rows() || _row_vec.size() != columns()`, respectively.
		 * \complexity Linear in `columns()` plus linear in distance between
		 *             `_row_pos` and `end` of the container.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no changes in the container.
	 	 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> iterator insert_row(size_type _row_pos, const std::vector<value_type>& _row_vec) {
			if (_row_pos > rows_)
				throw std::out_of_range("_row_pos must be <= current value of rows().");
			if (_row_vec.size() != cols_)
				throw std::invalid_argument("_row_vec.size() must = current value of columns().");
			++rows_;
			return mtx.insert(mtx.cbegin() + _row_pos*cols_, _row_vec.cbegin(), _row_vec.cend());
		}
		/**
		 * \brief Inserts a row vector to the position one slot before `_row_pos` using move-semantics.
		 *
		 * If `_row_vec.size() < columns()` then `_row_vec` is resized locally before being move-inserted. This method
		 * is also the "default-row-inserter" as the `_row_vec` parameter is optional - if no argument is provided
		 * then a vector of default-inserted values is used as the newly inserted row.
		 *
		 * \warning Attempting to insert a row vector with size greater than current `columns()` size results
		 *          in an exception being thrown - a call to columns_resize() passing a size greater than or
		 *          equal to `_row_vec.size()` is required before calling this method to successfully insert
		 *          `_row_vec` to the matrix whilst expanding the size of each row vector. 
		 * \param _row_pos Position one slot after insertion point.
		 * \param _row_vec rvalue reference to instance of `std::vector` to move-insert.
		 * \return Iterator pointing to the first element inserted.
		 * \throw Throws `std::out_of_range` exception, `std::invalid_argument` exception 
		 *        if `_row_pos > rows() || _row_vec.size() > columns()`, respectively.
		 * \complexity If `_row_vec.size() == columns()` then linear in `columns` plus linear in distance between
		 *             `_row_pos` and `end` of the container, else if `_row_vec.size() < columns()` then linear in
		 *             `columns()` plus linear in distance between `_row_pos` and `end` of the container plus linear
		 *             in `columns() - _row_vec.size()`.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no changes in the container.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_move_assignable<_Uty>::value>
		> iterator insert_row(size_type _row_pos, std::vector<value_type>&& _row_vec = std::vector<value_type>()) {
			if (_row_pos > rows_)
				throw std::out_of_range("_row_pos must be <= current value of rows().");
			if (_row_vec.size() > cols_)
				throw std::invalid_argument("_row_vec.size() must be <= current value of columns().");
			++rows_;
			// move-insert _row_vec contents to specified row position
			if (_row_vec.size() < cols_)
				_row_vec.resize(cols_);
			return mtx.insert(mtx.cbegin() + _row_pos*cols_, 
				std::make_move_iterator(_row_vec.begin()), std::make_move_iterator(_row_vec.end()));
		}
		/**
		 * \brief Inserts a column vector to the position one slot before `_col_pos` where each
		 *        element in the inserted column will have the specified value `_val`.
		 *
		 * \param _col_pos Position one slot after insertion point.
		 * \param _val Value to initialise all elements of the newly inserted column with.
		 * \return Iterator pointing to the first element inserted.
		 * \throw Throws `std::invalid_argument` exception if `_col_pos > columns()`.
		 * \complexity Linear in `rows()` multiplied by linear in distance between `_col_pos` and 
		 *             `end` of the container.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> iterator insert_column(size_type _col_pos, const value_type& _val) {
			return insert_column(_col_pos, std::move(std::vector<value_type>(rows_, _val)));
		}
		/**
		 * \brief Inserts a column vector to the position one slot before `_col_pos`.
		 *
		 * The column-vector inserted must have `size() == rows()` otherwise an exception is thrown. Use 
		 * `_col_vec.resize(rows())` before insertion if `_col_vec.size() < rows()` or resize this container
		 * via `rows_resize(_col_vec.size())` to accommodate the extra rows required by the column-vector
		 * to be inserted.
		 *
		 * \warning Attempting to insert a column vector with size greater than current `rows()` size
		 *          results in an exception being thrown - a call to rows_resize() passing a size greater
		 *          than or equal to `_col_vec.size()` is required before calling this method to successfully
		 *          insert `_col_vec` to the matrix whilst expanding the size of each column vector.
		 * \param _col_pos Position one slot after insertion point.
		 * \param _row_vec Instance of `std::vector` column to insert.
		 * \return Iterator pointing to the first element inserted.
		 * \throw Throws `std::out_of_range` exception, `std::invalid_argument` exception 
		 *        if `_col_pos > columns() || _col_vec.size() != rows()`, respectively.
		 * \complexity Linear in `rows()` multiplied by linear in distance
		 *             between `_cols_pos` and `end` of the container.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> iterator insert_column(size_type _col_pos, const std::vector<value_type>& _col_vec) {
			if (_col_pos > cols_)
				throw std::out_of_range("_col_pos must be <= current value of columns().");
			if (_col_vec.size() != rows_)
				throw std::invalid_argument("_col_vec.size() must = current value of rows().");
			iterator rtn; // val to return
			// no. of elements in _col_vec matches row size, insert each value of
			// _col_vec sequentially into new column of matrix
			for (size_type i = 0; i < _col_vec.size(); ++i) {
				// insert i'th el. of _col_vec at position offset from
				// beginning by row, col position plus loop index to take
				// account for extra values inserted on previous iteration
				if (!i) rtn = mtx.insert(mtx.cbegin() + i*(cols_ + 1) + _col_pos, _col_vec[i]);
				else mtx.insert(mtx.cbegin() + i*(cols_ + 1) + _col_pos, _col_vec[i]);
			}
			++cols_;
			return rtn;
		}
		/**
		 * \brief Inserts a column vector to the position one slot before `_col_pos` using move-semantics.
		 *
		 * If `_col_vec.size() < rows()` then `_col_vec` is resized locally before being move-inserted. This method
		 * is also the "default-column-inserter" as the `_row_vec` parameter is optional - if no argument is provided
		 * then a vector of default-inserted values is used as the newly inserted column.
		 *
		 * \warning Attempting to insert a column vector with size greater than current `rows()` size
		 *          results in an exception being thrown - a call to rows_resize() passing a size greater
		 *          than or equal to `_col_vec.size()` is required before calling this method to successfully
		 *          insert `_col_vec` to the matrix whilst expanding the size of each column vector.
		 * \param _col_pos Position one slot after insertion point.
		 * \param _col_vec rvalue reference to instance of `std::vector` to move-insert.
		 * \return Iterator pointing the the first element inserted.
		 * \throw Throws `std::out_of_range` exception, `std::invalid_argument` exception
		 *        if `_col_pos > columns() || _col_vec.size() > rows()`, respectively.
		 * \complexity If `_col_vec.size() == rows()` then linear in `rows()` multiplied by linear in distance
		 *             between `_cols_pos` and `end` of the container, else if `_col_vec.size() < rows()` then
		 *             linear in `rows()` multiplied by linear in distance between `_col_pos` and `end` of the 
		 *             container plus linear in `rows() - _col_vec.size()`.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_move_assignable<_Uty>::value>
		> iterator insert_column(size_type _col_pos, std::vector<value_type>&& _col_vec = std::vector<value_type>()) {
			if (_col_pos > cols_)
				throw std::out_of_range("_col_pos must be <= current value of columns().");
			if (_col_vec.size() > rows_)
				throw std::invalid_argument("_col_vec.size() must be <= current value of rows().");
			iterator rtn; // val to return
			if (_col_vec.size() < rows_)
				_col_vec.resize(rows_);
			// move-insert _col_vec contents to specified column position
			for (size_type i = 0; i < _col_vec.size(); ++i) {
				if (!i) rtn = mtx.insert(mtx.cbegin() + i*(cols_ + 1) + _col_pos, std::move(_col_vec[i]));
				else mtx.insert(mtx.cbegin() + i*(cols_ + 1) + _col_pos, std::move(_col_vec[i]));
			}
			++cols_;
			return rtn;
		}
		/**
		 * \brief Erases a row vector at `_row_pos`.
		 *
		 * \param _row_pos Row position to remove.
		 * \return Iterator following the last removed element, i.e. the iterator pointing to
		 *         to the first element of the next row or `end()` if last row was erased.
		 * \throw Throws `std::out_of_range` exception if `!(_row_pos < rows())`.
		 * \complexity Linear in `columns()` plus linear in distance between last element of
		 *             the row and `end` of the container.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_move_assignable<_Uty>::value>
		> iterator erase_row(size_type _row_pos) {
			if (!(_row_pos < rows_))
				throw std::out_of_range("_row_pos must be < current value of rows().");
			--rows_;
			return mtx.erase(mtx.cbegin() + _row_pos*cols_, mtx.cbegin() + cols_*(_row_pos + 1));
		}
		/**
		 * \brief Erases a column vector at `_col_pos`.
		 *
		 * \param _col_pos Column position to remove.
		 * \return Iterator following the last removed element, i.e. the iterator pointing
		 *         to the next element along from the last row of the erased column.
		 * \throw Throws `std::out_of_range` exception if `!(_col_pos < columns())`.
		 * \complexity Linear in `rows()` multiplied by linearly decreasing factor given
		 *             by distance between each element in column and `end` of container.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_move_assignable<_Uty>::value>
		> iterator erase_column(size_type _col_pos) {
			if (!(_col_pos < cols_))
				throw std::out_of_range("_col_pos must be < current value of columns().");
			iterator rtn;
			// erase el. of mtx at position offset from beginning by
			// row, col position minus loop index to take account for
			// values erased on previous iteration
			for (size_type i = 0; i < rows_; ++i)
				rtn = mtx.erase(mtx.cbegin() + i*(cols_ - 1) + _col_pos);
			--cols_;
			return rtn;
		}
		/**
		 * \brief Assigns the given value `_val` to all elements in the container.
		 *
		 * \param _val Value to assign to all elements.
		 * \complexity Exactly `rows()*columns()` assignments.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> void fill(const value_type& _val) noexcept {
			std::fill(mtx.begin(), mtx.end(), _val);
		}
		/**
		 * \brief Pushes an extra row-vector to the back of the container where each element
		 *        in the inserted row will have the specified value `_val`.
		 *
		 * \remark Equivalent to `insert_row(rows(), _val)`.
		 * \param _val Value to initialise all elements of the newly inserted row with.
		 * \complexity Amortized linear in `columns()`.
		 * \exceptionsafety If `_Ty`'s move constructor is not `noexcept` and `_Ty` is not 
		 *                  `CopyInsertable` into `*this`, `dynamic_matrix` will use the throwing
		 *                  move constructor. If it throws, any guarantee is waived and the effects
		 *                  are unspecified. Otherwise, there is a strong guarantee (if an exception
		 *                  is thrown there are no changes in the container).
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> void push_row(const value_type& _val) {
			for (size_type i = 0; i < cols_; ++i)
				mtx.push_back(_val);
			++rows_;
		}
		/**
		 * \brief Pushes an extra row-vector to the back of the container.
		 *
		 * The row-vector pushed must have `size() == columns()` otherwise an exception is thrown. Use 
		 * `_row_vec.resize(columns())` before pushing if `_row_vec.size() < columns()` or resize this
		 * container via `columns_resize(_row_vec.size())` to accomodate the extra columns required by the
		 * row-vector to be pushed.
		 *
		 * \remark Equivalent to `insert_row(rows(), _row_vec)`.
		 * \param _row_vec Instance of `std::vector` row to insert.
	 	 * \throw Throws `std::invalid_argument` exception if `_row_vec.size() != columns()`.
		 * \complexity Amortized linear in `columns()`.
		 * \exceptionsafety If `_Ty`'s move constructor is not `noexcept` and `_Ty` is not
		 *                  `CopyInsertable` into `*this`, `dynamic_matrix` will use the throwing
		 *                  move constructor. If it throws, any guarantee is waived and the effects
		 *                  are unspecified. Otherwise, there is a strong guarantee (if an exception
		 *                  is thrown there are no changes in the container).
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> void push_row(const std::vector<value_type>& _row_vec) {
			if (_row_vec.size() != cols_)
				throw std::invalid_argument("_row_vec.size() must = current value of columns().");
			// push_back each element of _row_vec to form new row
			for (const auto& el : _row_vec) {
				mtx.push_back(el);
			}
			++rows_;
		}
		/**
		 * \brief Pushes an extra row-vector to the back of the container using move-semantics.
		 *
		 * If `_row_vec.size() < columns()` then `_row_vec` is resized locally before being pushed back. This
		 * method is also the "default-row-push-back" as the `_row_vec` paramater is optional - if no argument
		 * is provided then a vector of default-inserted values is used as the newly pushed back row.
		 *
		 * \remark Equivalent to `insert_row(rows(), std::move(_row_vec))`.
		 * \param _row_vec rvalue reference to instance of `std::vector` to push-back via moving each element.
		 * \throw Throws `std::invalid_argument` exception if `_row_vec.size() > columns()`.
		 * \complexity If `_row_vec.size() == columns()` then amortized linear in `columns()`, else
		 *             if `_row_vec.size() < columns()` then amortized linear in `columns()` plus
		 *             linear in `columns() - _row_vec.size()`.
		 * \exceptionsafety If `_Ty`'s move constructor is not `noexcept` and `_Ty` is not
		 *                  `CopyInsertable` into `*this`, `dynamic_matrix` will use the throwing
		 *                  move constructor. If it throws, any guarantee is waived and the effects
		 *                  are unspecified. Otherwise, there is a strong guarantee (if an exception
		 *                  is thrown there are no changes in the container).
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_move_assignable<_Uty>::value>
		> void push_row(std::vector<value_type>&& _row_vec = std::vector<value_type>()) {
			if (_row_vec.size() > cols_)
				throw std::invalid_argument("_row_vec.size() must be <= current value of columns().");
			if (_row_vec.size() < cols_)
				_row_vec.resize(cols_);
			// push_back each element of _row_vec to form new row
			for (auto& el : _row_vec)
				mtx.push_back(std::move(el));
			++rows_;
		}
		/**
		 * \brief Pushes an extra column-vector to the back of the container where each element
		 *        in the inserted column will have the specified value `_val`.
		 *
		 * \remark Equivalent to `insert_column(columns(), _val)`.
		 * \param _val Value to initialise all elements of the newly inserted column with.
		 * \complexity Amortized linear in `rows()`.
		 * \exceptionsafety If an exception is thrown when inserting a single element at the `end`, and `_Ty`
		 *                  is `CopyInsertable` or `std::is_nothrow_most_constructible<_Ty>::value == true`,
		 *                  there is a strong guarantee (no changes in the container).
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> void push_column(const value_type& _val) {
			insert_column(cols_, _val);
		}
		/**
		 * \brief Pushes an extra column-vector to the back of the container.
		 *
		 * If `_col_vec.size() < rows()` then `_col_vec` is copied locally and resized to
		 * `rows()` before being pushed back (preserving the class invarant).
		 *
		 * \remark Equivalent to `insert_column(columns(), _col_vec)`.
		 * \param _col_vec Instance of `std::vector` column to insert.
		 * \throw Throws `std::invalid_argument` exception if `_col_vec.size() > rows()`.
		 * \complexity If `_col_vec.size() == rows()` then amortized linear in `rows()`, else if
		 *             `_col_vec.size() < rows()` then amortized linear in `rows()` plus linear in
		 *             `rows() - _col_vec.size()`.
		 * \exceptionsafety If an exception is thrown when inserting a single element at the `end`, and `_Ty`
		 *                  is `CopyInsertable` or `std::is_nothrow_move_constructible<_Ty>::value == true`,
		 *                  there is a strong guarantee (no changes in the container). Additionally, if
		 *                  `std::invalid_argument` exception is thrown there is also a strong guarantee.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> void push_column(const std::vector<value_type>& _col_vec) {
			insert_column(cols_, _col_vec);
		}
		/**
		 * \brief Pushes an extra column-vector to the back of the container using move-semantics.
		 *
		 * If `_col_vec.size() < rows()` then `_col_vec` is resized locally before being pushed back.  This
		 * method is also the "default-column-push-back" as the `_row_vec` paramater is optional - if no argument
		 * is provided then a vector of default-inserted values is used as the newly pushed back column.
		 *
		 * \remark Equivalent to `insert_column(columns(), std::move(_col_vec))`.
		 * \param _col_vec rvalue reference to instance of `std::vector` column to insert via move-semantics.
		 * \throw Throws `std::invalid_argument` exception if `_col_vec.size() > rows()`.
		 * \complexity If `_col_vec.size() == rows()` then amortized linear in `rows()`, else if
		 *             `_col_vec.size() < rows()` then amortized linear in `rows()` plus linear in
		 *             `rows() - _col_vec.size()`.
		 * \exceptionsafety If an exception is thrown when inserting a single element at the `end`, and `_Ty`
		 *                  is `CopyInsertable` or `std::is_nothrow_move_constructible<_Ty>::value == true`,
		 *                  there is a strong guarantee (no changes in the container). Additionally, if
		 *                  `std::invalid_argument` exception is thrown there is also a strong guarantee.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_move_assignable<_Uty>::value>
		> void push_column(std::vector<value_type>&& _col_vec = std::vector<value_type>()) {
			insert_column(cols_, std::move(_col_vec));
		}
		/**
		 * \brief Pops the last row from the back of the container.
		 *
		 * \remark Equivalent to `erase_row(rows() - 1)`.
		 * \complexity Linear in `columns()`. 
		 * \exceptionsafety If container is `empty()` then no-throw guarantee, otherwise
		 *                  undefined behaviour.
		 */
		void pop_row() {
			for (size_type i = 0; i < cols_; ++i)
				mtx.pop_back();
			--rows_;
		}
		/**
		 * \brief Pops the last column from the back of the container.
		 *
		 * \remark Equivalent to `erase_columns(columns() - 1)`.
		 * \complexity Linear in `rows()` multiplied by linearly decreasing factor given
		 *             by distance between each element in column and `end` of container.
		 * \exceptionsafety No-throw guarantee unless an exception is thrown by the copy constructor, 
		 *                  move constructor, copy-assignment operator or move-assignment operator of
		 *                  `_Ty`.
		 */
		void pop_column() {
			erase_column(cols_ - 1);
		}
		/**
		 * \brief Resizes the container to contain `_rows` row vectors, where any extra values added
		 *       (if any) are initialised according to the default constructor of `value_type`.
		 *
		 * If `_rows > rows()` the container is expanded to include `rows() - _rows` extra rows 
		 * with each value in the new row vectors initialised via default-insertion of `value_type`.
		 * If `_rows < rows()` the container is contracted by `rows() - _rows` rows such that only the
		 * first `_rows` rows of the container remain. If `_rows == rows()` this method does nothing.
		 *
		 * \param _rows New number of rows in the container.
		 * \complexity Linear in `|_rows - rows()|` multiplied by linear in `columns()`.
		 * \exceptionsafety If `_rows > rows()` then no-throw guarantee, else if `_rows < rows()` and
		 *                  the container is `empty()` then undefined behaviour otherwise no-throw
		 *                  guarantee.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_move_assignable<_Uty>::value
				&& std::is_default_constructible<_Uty>::value>
		> void rows_resize(size_type _rows) {
			size_type tmp_rows = rows_;
			if (_rows == rows_) return;
			if (_rows > rows_) {	// expand number of rows in matrix
				for (size_type i = 0; i < (_rows - tmp_rows); ++i)
					insert_row(rows_, std::move(std::vector<value_type>(cols_)));
			}
			else {	// contract number of rows in matrix
				for (size_type i = 0; i < (tmp_rows - _rows); ++i)
					pop_row();
			}
		}
		/**
		 * \brief Resizes the container to contain `_rows` row vectors, where any extra values
		 *        added (if any) are initialised with `_val`.
		 *
		 * If `_rows > rows()` the container is expanded to include `rows() - _rows` extra rows
		 * with each value in the new row vectors initialised with `_val`. If `_rows < rows()` the
		 * container is contracted by `rows() - _rows` rows such that only the first `_rows` rows of
		 * the container remain. If `_rows == rows()` this method does nothing.
		 *
		 * \param _rows New number of rows in the container.
		 * \param _val Value to initialise all elements of new row vectors with (if any).
		 * \complexity Linear in `|_rows - rows()|` multiplied by linear in `columns()`.
		 * \exceptionsafety If `_rows > rows()` then no-throw guarantee, else if `_rows < rows()` and
		 *                  the container is `empty()` then undefined behaviour otherwise no-throw
		 *                  guarantee.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> void rows_resize(size_type _rows, const value_type& _val) {
			size_type tmp_rows = rows_;
			if (_rows == rows_) return;
			if (_rows > rows_) {	// expand number of rows in matrix
				for (size_type i = 0; i < (_rows - tmp_rows); ++i) 
					insert_row(rows_, _val);
			}
			else {	// contract number of rows in matrix
				for (size_type i = 0; i < (tmp_rows - _rows); ++i)
					pop_row();
			}
		}
		/**
		 * \brief Resizes the container to contain `_cols` column vectors, where any extra values added
		 *        (if any) are initialised according to the default constructor of `value_type`.
		 *
		 * If `_cols > columns()` the container is expanded to include `columns() - _cols` extra columns
		 * with each value in the new column vectors initialised via default-insertion of `value_type`.
		 * If `_cols < columns()` the container is contracted by `columns() - _cols` columns such that
		 * only the first `_cols` columns of the container remain. If `_cols == columns()` this method 
		 * does nothing.
		 *
		 * \param _cols New number of columns in the container.
		 * \complexity If `_cols > columns()` then linear in `|_cols - columns()|` multiplied by amortized
		 *             linear in `rows()`, else if `_cols < columns()` then linear in `|columns() - _cols|`
		 *             multiplied by complexity of `pop_column()`.
		 * \exceptionsafety If `_cols > columns()` then no-throw guarantee, else if `_cols < columns()`
		 *                  and the container is `empty()` then undefined behaviour otherwise no-throw
		 *                  guarantee.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_move_assignable<_Uty>::value
				&& std::is_default_constructible<_Uty>::value>
		> void columns_resize(size_type _cols) {
			size_type tmp_cols = cols_;
			if (_cols == cols_) return;
			if (_cols > cols_) {	// expand number of columns in matrix
				for (size_type i = 0; i < (_cols - tmp_cols); ++i)
					insert_column(cols_, std::move(std::vector<value_type>(rows_)));
			}
			else {	// contract number of columns in matrix
				for (size_type i = 0; i < (tmp_cols - _cols); ++i)
					pop_column();
			}
		}
		/**
		 * \brief Resizes the container to contain `_cols` column vectors, with any extra values
		 *        added (if any) being initialised with `_val`.
		 *
		 * If `_cols > columns()` the container is expanded to include `columns() - _cols` extra columns
		 * with each value in the new column vectors initialised with `_val`. If `_cols < columns()` the
		 * container is contracted by `columns() - _cols` columns such that only the first `_cols` columns
		 * of the container remain. If `_cols == columns()` this method does nothing.
		 *
		 * \param _cols New number of columns in the container.
		 * \param _val Value to initialise all elements of new column vectors with (if any).
		 * \complexity If `_cols > columns()` then linear in `|_cols - columns()|` multiplied by amortized
		 *             linear in `rows()`, else if `_cols < columns()` then linear in `|columns() - _cols|` 
		 *             multiplied by complexity of `pop_column()`.
		 * \exceptionsafety If `_cols > columns()` then no-throw guarantee, else if `_cols < columns()`
		 *                  and the container is `empty()` then undefined behaviour otherwise no-throw
		 *                  guarantee.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> void columns_resize(size_type _cols, const value_type& _val) {
			size_type tmp_cols = cols_;
			if (_cols == cols_) return;
			if (_cols > cols_) {	// expand number of columns in matrix
				for (size_type i = 0; i < (_cols - tmp_cols); ++i)
					insert_column(cols_, _val);
			}
			else {	// contract number of columns in matrix
				for (size_type i = 0; i < (tmp_cols - _cols); ++i)
					pop_column();
			}
		}
		/**
		 * \brief Resizes the container to contain `_rows` row vectors and `_cols` column vectors where
		 *        any extra values added (if any) are initialised according to the default-constructor
		 *        of `value_type`.
		 *
		 * \param _rows New number of rows in the container.
		 * \param _cols New number of columns in the container.
		 * \complexity Complexity of `rows_resize(_rows)` plus complexity of `columns_resize(_cols)`.
		 * \exceptionsafety See exception-safeties of `rows_resize` and `columns_resize`.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_move_assignable<_Uty>::value
				&& std::is_default_constructible<_Uty>::value>
		> void resize(size_type _rows, size_type _cols) {
			rows_resize(_rows);
			columns_resize(_cols);
		}
		/**
		 * \brief Resizes the container to contain `_rows` row vectors and `_cols` column vectors where
		 *        any extra values added (if any) are initialised with `_val`.
		 *
		 * \see rows_resize
		 * \see columns_resize
		 * \param _rows New number of rows in the container.
		 * \param _cols New number of columns in the container.
		 * \param _val Value to initialise all elements of new row and column vectors with (if any).
		 * \complexity Complexity of `rows_resize(_rows, _val)` plus complexity
		 *             of `columns_resize(_cols, _val)`.
		 * \exceptionsafety See exception-safeties of `rows_resize` and `columns_resize`.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> void resize(size_type _rows, size_type _cols, const value_type& _val) {
			rows_resize(_rows, _val);
			columns_resize(_cols, _val);
		}
		/**
		 * \brief Exchanges the contents of the container with those of `_other`. Does not cause iterators
		 *        and references to associate with the other container.
		 *
	 	 * \param _other `dynamic_matrix` container to swap with.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee.
		 */
		void swap(dynamic_matrix& _other) {
			mtx.swap(_other.mtx);
			std::swap(rows_, _other.rows_);
			std::swap(cols_, _other.cols_);
		}
		/**
		 * \brief Exchanges the contents of two `crsc::dynamic_matrix` containers. Does not cause 
		 *        iterators and references to associate with the other containers.
		 *
		 * \param lhs First instance of `dynamic_matrix`.
		 * \param rhs Second instance of `dynamic_matrix`.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee.
		 */
		static void swap(dynamic_matrix& lhs, dynamic_matrix& rhs) {
			lhs.swap(rhs);
		}
		/**
		 * \brief Erases the specified row and column from the container, yielding the submatrix.
		 *
		 * \param _row_index Index of row to remove.
		 * \param _col_index Index of column to remove.
		 * \return `*this`.
		 * \complexity Complexity of `erase_row(_row_index)` plus complexity of `erase_column(_col_index)`.
		 * \exceptionsafety See exception safeties of `erase_row` and `erase_column`.
		 */
		dynamic_matrix& submatrix(size_type _row_index, size_type _col_index) {
			erase_row(_row_index);
			erase_column(_col_index);
			return *this;
		}
		// OVERLOADED OPERATORS
		/**
		 * \brief Checks for equality of this container and `_other`.
		 *
		 * \param _other Container to check for equality.
		 * \return `true` if `*this` equals, element-wise `_other`, otherwise `false`.
		 * \complexity Constant if `rows() != _other.rows() || columns != _other.columns()`,
		 *             or if `this == &_other`, otherwise linear in `rows()*columns()`.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		bool operator==(const dynamic_matrix& _other) const noexcept {
			if (this != &_other) {
				if (rows_ != _other.rows_ || cols_ != _other.cols_)
					return false;
				return std::equal(mtx.begin(), mtx.end(), _other.mtx.begin());
			}
			return true;
		}
		/**
		 * \brief Checks for inequality of this container and `_other`.
		 *
		 * \param _other Container to check for inequality.
		 * \return `true` if `*this` unequals, element-wise `_other`, otherwise `false`.
		 * \complexity Constant if `rows() != _other.rows() || columns != _other.columns()`,
		 *             otherwise linear in `rows()*columns()`.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		bool operator!=(const dynamic_matrix& _other) const noexcept {
			return !(*this == _other);
		}
	private:
		std::vector<value_type, allocator_type> mtx;
		size_type rows_;
		size_type cols_;
	};
	/**
	 * \brief Stream insertion operator. Inserts formatted `dynamic_matrix` contents to a `std::ostream`.
	 *
	 * \param _os Instance of `std::ostream` to write to.
	 * \param _dm `dynamic_matrix` object to write to stream.
	 * \return Modified reference to `_os` containing the container data.
	 * \complexity Linear in `rows()*columns()`.
	 * \exceptionsafety No-throw guarantee, `noexcept` specification.
	 */
	template<typename _Ty,
		class _Alloc = std::allocator<_Ty>,
		class = std::enable_if_t<has_insertion_operator<_Ty>::value>
	> std::ostream& operator<<(std::ostream& os, const dynamic_matrix<_Ty, _Alloc>& dm) {
		typename dynamic_matrix<_Ty, _Alloc>::size_type count = 0;
		for (const auto& el : dm) {
			os << el << ' ';
			++count;
			if (!(count % dm.columns()))
				os << '\n';
		}
		return os;
	}
	/**
	 * \brief Makes an identity `dynamic_matrix` of specified size.
	 *
	 * \tparam _Ty Type of stored elements.
	 * \tparam _Alloc An allocator that is used to acquire memory to store the elements. The type must meet the requirements
	 *                of `Allocator` (see C++ Standard). Behaviour is undefined if `_Alloc::value_type != _Ty`.
	 * \remark Only enabled if `std::is_arithmetic<_Ty>::value`.
	 * \param _rows Number of rows.
	 * \param _cols Number of columns.
	 * \param alloc Allocator to use for all memory allocations of this container.
	 * \return Identity `dynamic_matrix` of given dimensions.
	 * \throw Throws `std::logic_error` if `_rows != _cols`.
	 * \complexity Linear in `_rows*_cols` plus complexity of container's copy constructor (subject to RVO).
	 * \exceptionsafety See exception safeties of `dynamic_matrix(_rows, _cols, alloc)` constructor and copy constructor.
	 */
	template<typename _Ty,
		class _Alloc = std::allocator<_Ty>,
		class = std::enable_if_t<std::is_arithmetic<_Ty>::value>
	> dynamic_matrix<_Ty, _Alloc> make_identity_matrix(std::size_t _rows, std::size_t _cols, const _Alloc& alloc = _Alloc()) {
		if (_rows != _cols)
			throw std::logic_error("identity_matrix must have _rows == _cols.");
		dynamic_matrix<_Ty, _Alloc> identity_matrix(_rows, _cols, alloc);
		for (std::size_t i = 0; i < _rows; ++i) {
			for (std::size_t j = 0; j < _cols; ++j)
				if (i == j) identity_matrix[i][j] = static_cast<_Ty>(1);
		}
		return identity_matrix;
	}
	/**
	 * \brief Converts a two dimensional C-style array `arr_2d` to a `dynamic_matrix`, deleting `arr_2d` in the process.
	 *
	 * This method initialises a `dynamic_matrix` with `arr_2d` and then deletes `arr_2d` from memory, therefore if `arr_2d`
	 * was not allocated via heap storage then do not use this method - use `crsc::make_dynamic_matrix` instead.
	 *
	 * \tparam _Ty Type of stored elements.
	 * \tparam _Alloc An allocator that is used to acquire memory to store the elements. The type must meet the requirements
	 *                of `Allocator` (see C++ Standard). Behaviour is undefined if `_Alloc::value_type != _Ty`.
	 * \param arr_2d Two-dimensional C-style array used as source to initialise elements of the container with, deleted after use.
	 * \param rows Number of rows.
	 * \param cols Number of columns.
	 * \param alloc Allocator to use for all memory allocations of this container.
	 * \return A `dynamic_matrix` object constructed using the contents of `_arr_2d`.
	 * \complexity Linear in `_rows*_cols` plus complexity of container's copy constructor (subject to RVO).
	 * \exceptionsafety See exception safeties of `dynamic_matrix(_arr_2d, _row, _cols, alloc)` constructor and copy constructor,
	 *                  additionally undefined behaviour if either of `_rows`, `_cols` is not equal to rows, columns of `_arr_2d`.
	 */
	template<typename _Ty,
		class _Alloc = std::allocator<_Ty>
	> dynamic_matrix< _Ty, _Alloc> to_dynamic_matrix(_Ty** arr_2d, std::size_t rows, std::size_t cols, const _Alloc& alloc = _Alloc()) {
		dynamic_matrix<_Ty, _Alloc> dynmtx(arr_2d, rows, cols, alloc);
		for (std::size_t i = 0; i < rows; ++i)
			delete[] arr_2d[i];
		delete[] arr_2d;
		return dynmtx;
	}
	/**
	 * \brief Makes a `dynamic_matrix` from a two dimensional C-style array `arr_2d`.
	 *
	 * This method initialises a `dynamic_matrix` with `arr_2d` but does not delete `arr_2d` from memory upon matrix
	 * initialisation, if `arr_2d` was allocated via heap storage and it is no longer required after being used to 
	 * construct a `dynamic_matrix` then use `crsc::to_dynamic_matrix` instead.
	 *
	 * \tparam _Ty Type of stored elements.
	 * \tparam _Alloc An allocator that is used to acquire memory to store the elements. The type must meet the requirements
	 *                of `Allocator` (see C++ Concepts). Behaviour is undefined if `_Alloc::value_type != _Ty`.
	 * \param arr_2d Two-dimensional C-style array used as source to initialise elements of the container with.
	 * \param rows Number of rows.
	 * \param cols Number of columns.
	 * \param alloc Allocator to use for all memory allocations of this container.
	 * \return A `dynamic_matrix` object constructed using the contents of `arr_2d`.
	 * \complexity Linear in `rows*cols` plus complexity of container's copy constructor (subject to RVO).
	 * \exceptionsafety See exception safeties of `dynamic_matrix(_arr_2d, _row, _cols, alloc)` constructor and copy constructor,
	 *                  additionally undefined behaviour if either of `_rows`, `_cols` is not equal to rows, columns of `_arr_2d`.
	 */
	template<typename _Ty,
		class _Alloc = std::allocator<_Ty>
	> dynamic_matrix<_Ty, _Alloc> make_dynamic_matrix(_Ty** arr_2d, std::size_t rows, std::size_t cols, const _Alloc& alloc = _Alloc()) {
		return dynamic_matrix<_Ty, _Alloc>(arr_2d, rows, cols, alloc);
	}
	/**
	 * \brief Returns a `dynamic_matrix` whose elements equal the component-wise addition of `lhs` and `rhs`.
	 * 
	 * \param lhs First instance of `dynamic_matrix`.
	 * \param rhs Second instance of `dynamic_matrix`.
	 * \return Container consisting of sum of `lhs` and `rhs`.
	 * \throw Throws `std::invalid_argument` exception if `lhs.rows() != rhs.rows() ||
	 *        lhs.columns() != rhs.columns()`.
	 * \complexity Linear in `rows()*columns()` (assignments) plus linear in
	 *             `rows()*columns()` (additions).
	 */
	template<typename _Ty,
		class _Alloc = std::allocator<_Ty>
	> dynamic_matrix<_Ty, _Alloc> matrix_sum(const dynamic_matrix<_Ty, _Alloc>& lhs, const dynamic_matrix<_Ty, _Alloc>& rhs) {
		if (lhs.rows() != rhs.rows() || lhs.columns() != rhs.columns())
			throw std::invalid_argument("dynamic_matrix dimensions must agree for component-wise addition.");
		dynamic_matrix<_Ty, _Alloc> sum(lhs.rows(), lhs.columns());
		for (auto itlhs = lhs.begin(), itrhs = rhs.begin(), itsum = sum.begin(); itsum < sum.end(); ++itlhs, ++itrhs, ++itsum)
			*itsum = *itlhs + *itrhs;
		return sum;
	}
	/**
	 * \brief Returns a `dynamic_matrix` whose elements equal the component-wise subtraction of `rhs` from `lhs`.
	 *
	 * \param lhs First instance of `dynamic_matrix`.
	 * \param rhs Second instance of `dynamic_matrix`.
	 * \return Container consisting of difference of `lhs` and `rhs`.
	 * \throw Throws `std::invalid_argument` exception if `lhs.rows() != rhs.rows() ||
	 *        lhs.columns() != rhs.columns()`.
	 * \complexity Linear in `rows()*columns()` (assignments) plus linear in
	 *             `rows()*columns()` (subtractions).
	 */
	template<typename _Ty, 
		class _Alloc = std::allocator<_Ty>
	> dynamic_matrix<_Ty, _Alloc> matrix_difference(const dynamic_matrix<_Ty, _Alloc>& lhs, const dynamic_matrix<_Ty, _Alloc>& rhs) {
		if (lhs.rows() != rhs.rows() || lhs.columns() != rhs.columns())
			throw std::invalid_argument("dynamic_matrix dimensions must agree for component-wise subtraction.");
		dynamic_matrix<_Ty, _Alloc> difference(lhs.rows(), lhs.columns());
		for (auto itlhs = lhs.begin(), itrhs = rhs.begin(), itdiff = difference.begin(); itdiff < difference.end(); ++itlhs, ++itrhs, ++itdiff)
			*itdiff = *itlhs - *itrhs;
		return difference;
	}
	/**
	 * \brief Returns a `dynamic_matrix` which gives the matrix product of `lhs` with `rhs`.
	 *
	 * \param lhs First instance of `dynamic_matrix`.
	 * \param rhs Second instance of `dynamic_matrix`.
	 * \return Container consisting of product of `lhs` and `rhs`.
	 * \throw Throws `std::invalid_argument` exception if `lhs.columns() != rhs.rows()`.
	 * \complexity Linear in `lhs.rows()*rhs.columns()*lhs.columns()`.
	 */
	template<typename _Ty,
		class _Alloc = std::allocator<_Ty>
	> dynamic_matrix<_Ty, _Alloc> matrix_product(const dynamic_matrix<_Ty, _Alloc>& lhs, const dynamic_matrix<_Ty, _Alloc>& rhs) {
		if (lhs.columns() != rhs.rows())
			throw std::invalid_argument("dynamic_matrix dimensions must agree for matrix_product.");
		dynamic_matrix<_Ty, _Alloc> product(lhs.rows(), rhs.columns());
		for (std::size_t i = 0; i < product.rows(); ++i) {
			for (std::size_t j = 0; j < product.columns(); ++j) {
				for (std::size_t k = 0; k < lhs.columns(); ++k)
					product(i,j) += lhs(i,k) * rhs(k,j);
			}
		}
		return product;
	}
	/**
	 * \brief Computes the trace of a `dynamic_matrix` container instance `dm`.
	 *
	 * \param dm `dynamic_matrix` for which to compute the trace.
	 * \return Matrix trace of `dm`.
	 * \throw Throws `std::invalid_argument` exception if `dm.rows() != dm.columns()`.
	 * \complexity Linear in `dm.rows()`.
	 */
	template<typename _Ty,
		class _Alloc = std::allocator<_Ty>
	> _Ty matrix_trace(const dynamic_matrix<_Ty, _Alloc>& dm) {
		if (dm.rows() != dm.columns()) throw std::invalid_argument("cannot compute trace of non-square dynamic_matrix.");
		_Ty trace = _Ty();
		for (auto it = dm.begin(); it < dm.end(); std::advance(it, dm.columns() + 1))
			trace += *it;
		return trace;
	}
}

#endif // !DYNAMIC_MATRIX_H