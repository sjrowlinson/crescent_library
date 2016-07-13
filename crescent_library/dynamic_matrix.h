#ifndef DYNAMIC_MATRIX_H
#define DYNAMIC_MATRIX_H
#include <algorithm>
#include <iostream>
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
	 * access iteration and forward iteration is allowed. The order of iteration uses an "in-order traversal" such that elements
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
	 *         at compile-time then consider using crsc::matrix instead.
	 * \invariant Every row shall have an equal number of elements and every column shall have an equal number of elements
	 *            such that no holes occur in the structure.
	 * \author Samuel Rowlinson
	 * \date July, 2016
	 */
	template<typename _Ty,
		class _Alloc = std::allocator<_Ty>
	> class dynamic_matrix {
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
		 * \class proxy_row_vector
		 *
		 * \brief Proxy class used for enabling operator[][] overload on dynamic_matrix objects.
		 */
		class proxy_row_vector {
		public:
			proxy_row_vector(std::vector<value_type, _Alloc>& _vec, size_type _row_index, size_type _cols)
				: vec(_vec), row_index(_row_index), columns(_cols) {}
			const_reference operator[](size_type _col_index) const {
				return vec[row_index*columns + _col_index];
			}
			reference operator[](size_type _col_index) {
				return vec[row_index*columns + _col_index];
			}
		private:
			std::vector<value_type, _Alloc>& vec;
			size_type row_index;
			size_type columns;
		};
	public:
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
			: mtx(_rows*_cols, value_type(), alloc), rows_(_rows), cols_(_cols) {}	// temporarily using 3-arg constructor due to MSVC2015 bug
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
				dynamic_matrix(_other).swap(*this);
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
				dynamic_matrix(std::move(_other)).swap(*this);
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

		// Capacity

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

		// Element Access

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
				throw std::out_of_range("matrix indices out of bounds.");
			return mtx.at(_row_index*cols_ + _col_index);
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
				throw std::out_of_range("matrix indices out of bounds.");
			return mtx.at(_row_index*cols_ + _col_index);
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
		/**
		 * \brief Sends the container data to a `std::ostream` instance in a mathematical-matrix style format.
		 *
		 * \param _os Instance of `std::ostream` to write to.
	 	 * \param _delim Delimiter separating elements on each line of the `dynamic_matrix`.
		 * \return Modified reference to `_os` containing the container data.
		 * \complexity Linear in `rows()*columns()`.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		std::ostream& write(std::ostream& _os, char _delim = ' ') const noexcept {
			size_type count = 0;
			for (const auto& el : mtx) {
				_os << el << _delim;
				++count;
				if (!(count % cols_))
					_os << '\n';
			}
			return _os;
		}

		// Iterators

		/**
		 * \brief Returns a const_iterator the first element of the container.
		 *
		 * \remark If the container is empty, the return value will be equal to `cend()`.
		 * \return Constant iterator to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_iterator cbegin() const noexcept {
			return mtx.cbegin();
		}
		/**
		 * \brief Returns an iterator to the first element of the container.
		 *
		 * \remark If the container is empty, the return value will be equal to `end()`.
		 * \return Iterator to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		iterator begin() noexcept {
			return mtx.begin();
		}
		/**
		 * \brief Returns a const_iterator to the past-the-end element of the container.
		 *
		 * \return Constant iterator to the past-the-end element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_iterator cend() const noexcept {
			return mtx.cend();
		}
		/**
		 * \brief Returns an iterator to the past-the-end element of the container.
		 *
		 * \return Iterator to the past-the-end element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		iterator end() noexcept {
			return mtx.end();
		}
		/**
		 * \brief Returns a const_reverse_iterator to the first element of the reversed container. It
		 *        corresponds to the last element of the non-reversed container.
		 *
		 * \return Constant reverse iterator to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_reverse_iterator crbegin() const noexcept {
			return mtx.crbegin();
		}
		/**
		 * \brief Returns a reverse_iterator to the first element of the reversed container. It
		 *        corresponds to the last element of the non-reversed container.
		 *
		 * \return Reverse iterator to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		reverse_iterator rbegin() noexcept {
			return mtx.rbegin();
		}
		/**
		 * \brief Returns a const_reverse_iterator to the past-the-end element of the reversed container. It
		 *        corresponds to the element preceding the first element of the non-reversed container.
		 *
		 * \return Constance reverse iterator to the past-the-end element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_reverse_iterator crend() const noexcept {
			return mtx.crend();
		}
		/**
		 * \brief Returns a reverse_iterator to the past-the-end element of the reversed container. It
		 *        corresponds to the element preceding the first element of the non-reversed container.
		 *
		 * \return Reverse iterator to the past-the-end element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		reverse_iterator rend() noexcept {
			return mtx.rend();
		}

		// Operations/Modifiers

		/**
		 * \brief Removes all elements from the container but leaves the `capacity()` unchanged.
		 *
		 * \complexity Linear in `rows()*columns()`.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		void clear() noexcept {
			mtx.clear();
			rows_ = static_cast<size_type>(0);
			cols_ = static_cast<size_type>(0);
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
			class = std::enable_if_t<std::is_default_constructible<_Uty>::value>
		> iterator insert_row(size_type _row_pos, const value_type& _val = value_type()) {
			std::vector<value_type> row_vec(cols_, _val);
			return insert_row(_row_pos, row_vec);
		}
		/**
		 * \brief Inserts a row vector to the position one slot before `_row_pos`.
		 *
		 * If `_row_vec.size() < columns()` then `_row_vec` is copied locally and resized to
		 * `columns()` before being inserted (preserving the class invariant).
		 *
		 * \warning Attempting to insert a row vector with size greater than current `columns()` size results
		 *          in an exception being thrown - a call to columns_resize() passing a size greater than or
		 *          equal to `_row_vec.size()` is required before calling this method to successfully insert
		 *          `_row_vec` to the matrix whilst expanding the size of each row vector.
		 * \param _row_pos Position one slot after insertion point.
		 * \param _row_vec Instance of `std::vector` row to insert.
		 * \return Iterator pointing to the first element inserted.
		 * \throw Throws `std::invalid_argument` exception if `_row_pos > rows() || _row_vec.size() > columns()`.
		 * \complexity If `_row_vec.size() == columns()` then linear in `columns()` plus linear in distance between
		 *             `_row_pos` and `end` of the container, else if `_row_vec.size() < columns()` then linear in
		 *             `columns()` plus linear in distance between `_row_pos` and `end` of the container plus linear
		 *             in `columns() - _row_vec.size()`.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no changes in the container.
	 	 */
		iterator insert_row(size_type _row_pos, const std::vector<value_type>& _row_vec) {
			if (_row_pos > rows_)
				throw std::invalid_argument("_row_pos must be <= current value of rows().");
			if (_row_vec.size() > cols_)
				throw std::invalid_argument("_row_vec.size() must be <= current value of columns().");
			++rows_;
			// insert _row_vec contents to specified row position
			if (_row_vec.size() == cols_)
				return mtx.insert(mtx.cbegin() + _row_pos*cols_, _row_vec.cbegin(), _row_vec.cend());
			// if row vec size < cols_ then copy it and resize locally before inserting
			std::vector<value_type> resized = _row_vec;
			resized.resize(cols_);
			return mtx.insert(mtx.cbegin() + _row_pos*cols_, resized.cbegin(), resized.cend());
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
			class = std::enable_if_t<std::is_default_constructible<_Uty>::value>
		> iterator insert_column(size_type _col_pos, const value_type& _val = value_type()) {
			std::vector<value_type> col_vec(rows_, _val);
			return insert_column(_col_pos, col_vec);
		}
		/**
		 * \brief Inserts a column vector to the position one slot before `_col_pos`.
		 *
		 * If `_col_vec.size() < rows()` then `_col_vec` is copied locally and resized to
		 * `rows()` before being inserted (preserving the class invariant).
		 *
		 * \warning Attempting to insert a column vector with size greater than current `rows()` size
		 *          results in an exception being thrown - a call to rows_resize() passing a size greater
		 *          than or equal to `_col_vec.size()` is required before calling this method to successfully
		 *          insert `_col_vec` to the matrix whilst expanding the size of each column vector.
		 * \param _col_pos Position one slot after insertion point.
		 * \param _row_vec Instance of `std::vector` column to insert.
		 * \return Iterator pointing to the first element inserted.
		 * \throw Throws `std::invalid_argument` exception if `_col_pos > columns() || _col_vec.size() > rows()`.
		 * \complexity If `_col_vec.size() == rows()` then linear in `rows()` multiplied by linear in distance
		 *             between `_cols_pos` and `end` of the container, else if `_col_vec.size() < rows()` then
		 *             linear in `rows()` multiplied by linear in distance between `_col_pos` and `end` of the 
		 *             container plus linear in `rows() - _col_vec.size()`.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no changes
		 *                  in the container.
		 */
		iterator insert_column(size_type _col_pos, const std::vector<value_type>& _col_vec) {
			if (_col_pos > cols_)
				throw std::invalid_argument("_col_pos must be <= current value of columns().");
			if (_col_vec.size() > rows_)
				throw std::invalid_argument("_col_vec.size() must be <= current value of rows().");
			iterator rtn; // val to return
						  // no. of elements in _col_vec matches row size, insert each value of
						  // _col_vec sequentially into new column of matrix
			if (_col_vec.size() == rows_) {
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
			// copy _col_vec and resize to rows_ elements
			std::vector<value_type> resized = _col_vec;
			resized.resize(rows_);
			for (size_type i = 0; i < resized.size(); ++i) {
				// insert i'th el. of resized at position offset from
				// beginning by row, col position plus loop index to take
				// account for extra values inserted on previous iteration
				if (!i) rtn = mtx.insert(mtx.cbegin() + i*(cols_ + 1) + _col_pos, resized[i]);
				else mtx.insert(mtx.cbegin() + i*(cols_ + 1) + _col_pos, resized[i]);
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
		 * \throw Throws `std::invalid_argument` exception if `_row_pos >= rows()`.
		 * \complexity Linear in `columns()` plus linear in distance between last element of
		 *             the row and `end` of the container.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_move_assignable<_Uty>::value>
		> iterator erase_row(size_type _row_pos) {
			if (_row_pos >= rows_)
				throw std::invalid_argument("_row_pos must be < current value of rows().");
			--rows_;
			return mtx.erase(mtx.cbegin() + _row_pos*cols_, mtx.cbegin() + cols_*(_row_pos + 1));
		}
		/**
		 * \brief Erases a column vector at `_col_pos`.
		 *
		 * \param _col_pos Column position to remove.
		 * \return Iterator following the last removed element, i.e. the iterator pointing
		 *         to the next element along from the last row of the erased column.
		 * \throw Throws `std::invalid_argument` exception if `_col_pos >= columns()`.
		 * \complexity Linear in `rows()` multiplied by linearly decreasing factor given
		 *             by distance between each element in column and `end` of container.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_move_assignable<_Uty>::value>
		> iterator erase_column(size_type _col_pos) {
			if (_col_pos >= cols_)
				throw std::invalid_argument("_col_pos must be < current value of columns().");
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
		void fill(const value_type& _val) noexcept {
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
			class = std::enable_if_t<std::is_default_constructible<_Uty>::value>
		> void push_row(const value_type& _val = value_type()) {
			for (size_type i = 0; i < cols_; ++i)
				mtx.push_back(_val);
			++rows_;
		}
		/**
		 * \brief Pushes an extra row-vector to the back of the container.
		 *
		 * If `_row_vec.size() < columns()` then `_row_vec` is copied locally and resized to
		 * `columns()` before being pushed back (preserving the class invariant).
		 *
		 * \remark Equivalent to `insert_row(rows(), _row_vec).
		 * \param _row_vec Instance of `std::vector` row to insert.
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
		void push_row(const std::vector<value_type>& _row_vec) {
			if (_row_vec.size() > cols_)
				throw std::invalid_argument("_row_vec.size() must be <= current value of columns().");
			if (_row_vec.size() == cols_) {
				for (const auto& el : _row_vec) {
					mtx.push_back(el);
				}
			}
			else {
				std::vector<value_type> resized = _row_vec;
				resized.resize(cols_);
				for (const auto& el : resized) {
					mtx.push_back(el);
				}
			}
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
			class = std::enable_if_t<std::is_default_constructible<_Uty>::value>
		> void push_column(const value_type& _val = value_type()) {
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
		 *                  is `CopyInsertable` or `std::is_nothrow_most_constructible<_Ty>::value == true`,
		 *                  there is a strong guarantee (no changes in the container). Additionally, if
		 *                  `std::invalid_argument` exception is thrown there is also a strong guarantee.
		 */
		void push_column(const std::vector<value_type>& _col_vec) {
			insert_column(cols_, _col_vec);
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
		 * \brief Resizes the container to contain `_rows` row vectors, with any extra values
		 *        added (if any) being initialised with `_val`.
		 *
		 * If `_rows > rows()` the container is expanded to include `rows() - _rows` extra rows
		 * with each value in the new row vectors initialised with `_val`. If `_rows < rows()` the
		 * container is contracted by `rows() - _rows` rows such that only the first `_rows` rows of
		 * the container remain. If `_rows == rows()` this method does nothing.
		 *
		 * \param _rows New number of rows in the container.
		 * \param _val Value to initialise all elements of new row vectors with (if any).
		 * \complexity Linear in `_rows - rows()` multiplied by linear in `columns()`.
		 * \exceptionsafety If `_rows > rows()` then no-throw guarantee, else if `_rows < row()` and
		 *                  the container is `empty()` then undefined behaviour otherwise no-throw
		 *                  guarantee.
		 */
		void rows_resize(size_type _rows, const value_type& _val = value_type()) {
			size_type tmp_local_rows = rows_;
			if (_rows == tmp_local_rows) return;
			if (_rows > rows_) {
				for (size_type i = 0; i < (_rows - tmp_local_rows); ++i) 
					insert_row(rows_, _val);
			}
			else {
				for (size_type i = 0; i < (tmp_local_rows - _rows); ++i)
					pop_row();
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
		 * \complexity If `_cols > columns()` then linear in `_cols - columns()` multiplied by amortized
		 *             linear in `rows()`, else if `_cols < columns()` then linear in `columns() - _cols` 
		 *             multiplied by complexity of `pop_column()`.
		 * \exceptionsafety If `_cols > columns()` then no-throw guarantee, else if `_cols < columns()`
		 *                  and the container is `empty()` then undefined behaviour otherwise no-throw
		 *                  guarantee.
		 */
		void columns_resize(size_type _cols, const value_type& _val = value_type()) {
			size_type tmp_local_cols = cols_;
			if (_cols == tmp_local_cols) return;
			if (_cols > cols_) {
				for (size_type i = 0; i < (_cols - tmp_local_cols); ++i)
					insert_column(cols_, _val);
			}
			else {
				for (size_type i = 0; i < (tmp_local_cols - _cols); ++i)
					pop_column();
			}
		}
		/**
		 * \brief Resizes the container to contain `_rows` row vectors and `_cols` column vectors with
		 *        any extra values added (if any) being initialised with `_val`.
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
		void resize(size_type _rows, size_type _cols, const value_type& _val = value_type()) {
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
			size_type tmp_rows = rows_;
			size_type tmp_cols = cols_;
			rows_ = _other.rows_;
			cols_ = _other.cols_;
			_other.rows_ = tmp_rows;
			_other.cols_ = tmp_cols;
		}
		/**
		 * \brief Gets the submatrix of the container obtained by removing the specified row and column
		 *        and returning the resulting matrix.
		 *
		 * \param _row_index Index of row to remove.
		 * \param _col_index Index of column to remove.
		 * \return Submatrix of the container with specified row, column removed.
		 * \complexity Linear in `(rows()-1)*(columns()-1)` plus complexity of containers'
		 *             copy constructor (subject to RVO).
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		dynamic_matrix submatrix(size_type _row_index, size_type _col_index) const noexcept {
			dynamic_matrix<value_type> sub(rows_ - 1, cols_ - 1);
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
		/**
		 * \brief Erases the specified row and column from the container, yielding the submatrix.
		 *
		 * \param _row_index Index of row to remove.
		 * \param _col_index Index of column to remove.
		 * \return `*this`.
		 * \complexity Complexity of `erase_row(_row_index` plus complexity of `erase_column(_col_index)`.
		 * \exceptionsafety See exception safeties of `erase_row` and `erase_column`.
		 */
		dynamic_matrix& submatrix(size_type _row_index, size_type _col_index) {
			erase_row(_row_index);
			erase_column(_col_index);
			return *this;
		}
		/**
		 * \brief Computes the trace of a square `dynamic_matrix` container.
		 *
		 * \return The trace of the matrix-container.
		 * \throw Throws `std::logic_error` if `rows() != columns()`.
		 * \complexity Linear in `rows()`.
		 * \exceptionsafety Strong-guarantee - if an exception is thrown there are no changes 
		 *                  in the container.
		 */
		value_type trace() const {
			if (rows_ != cols_)
				throw std::logic_error("cannot compute trace() of non-square matrix.");
			value_type result = value_type();
			for (size_type i = 0; i < rows_; ++i)
				result += mtx[i*(cols_ + 1)];
			return result;
		}

		// Overloaded Operators

		dynamic_matrix& operator+=(const dynamic_matrix& _other) {
			if (rows_ != _other.rows_ || cols_ != _other.cols_)
				throw std::invalid_argument("dynamic_matrix dimensions must agree for addition.");
			for (size_type i = 0; i < rows_; ++i) {
				for (size_type j = 0; j < cols_; ++j)
					operator[](i)[j] += _other.at(i, j);
			}
			return *this;
		}
		dynamic_matrix& operator-=(const dynamic_matrix& _other) {
			if (rows_ != _other.rows_ || cols_ != _other.cols_)
				throw std::invalid_argument("dynamic_matrix dimensions must agree for subtraction.");
			for (size_type i = 0; i < rows_; ++i) {
				for (size_type j = 0; j < cols_; ++j)
					operator[](i)[j] -= _other.at(i, j);
			}
			return *this;
		}
		dynamic_matrix operator+(const dynamic_matrix& _other) const {
			if (rows_ != _other.rows_ || cols_ != _other.cols_)
				throw std::invalid_argument("dynamic_matrix dimensions must agree for addition.");
			dynamic_matrix<value_type> sum(rows_, cols_);
			for (size_type i = 0; i < rows_; ++i) {
				for (size_type j = 0; j < cols_; ++j)
					sum.at(i, j) = at(i, j) + _other.at(i, j);
			}
			return sum;
		}
		dynamic_matrix operator-(const dynamic_matrix& _other) const {
			if (rows_ != _other.rows_ || cols_ != _other.cols_)
				throw std::invalid_argument("dynamic_matrix dimensions must agree for subtraction.");
			dynamic_matrix<value_type> difference(rows_, cols_);
			for (size_type i = 0; i < rows_; ++i) {
				for (size_type j = 0; j < cols_; ++j)
					difference.at(i, j) = at(i, j) - _other.at(i, j);
			}
			return difference;
		}
		dynamic_matrix operator*(const dynamic_matrix& _other) const {
			if (cols_ != _other.rows_)
				throw std::invalid_argument("dynamic_matrix dimensions do not agree.");
			dynamic_matrix<value_type> product(rows_, _other.cols_);
			for (size_type i = 0; i < rows_; ++i) {
				for (size_type j = 0; j < _other.cols_; ++j) {
					for (size_type k = 0; k < cols_; ++k)
						product.at(i, j) += at(i, k) * _other.at(k, j);
				}
			}
			return product;
		}
		bool operator==(const dynamic_matrix& _other) const noexcept {
			if (rows_ != _other.rows_ || cols_ != _other.cols_)
				return false;
			if (this != &_other) {
				for (size_type i = 0; i < rows_; ++i) {
					for (size_type j = 0; j < cols_; ++j)
						if (at(i, j) != _other.at(i, j)) return false;
				}
			}
			return true;
		}
		bool operator!=(const dynamic_matrix& _other) const noexcept {
			return !(*this == _other);
		}

	private:
		std::vector<value_type, _Alloc> mtx;
		size_type rows_;
		size_type cols_;
	};

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
	 * \complexity Linear in `rows()*columns()` plus complexity of container's copy constructor (subject to RVO).
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

}

#endif /* DYNAMIC_MATRIX_H */