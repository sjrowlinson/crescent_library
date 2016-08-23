#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>

/**
 * \class dynamic_array_iterator
 *
 * \brief A bidirectional iterator used for the template container `dynamic_array`. Uses the
 *        `std::bidirectional_iterator_tag` and provides bidirectional iterator semantics -
 *        giving random access, forward and backward iterator support.
 *
 *  \tparam Ty The type of the stored elements.
 */
template<typename Ty>
class dynamic_array_iterator
	: public std::iterator<
	std::bidirectional_iterator_tag,
	Ty
	> {
public:
	// CONSTRUCTION/ASSIGNMENT
	dynamic_array_iterator(Ty* _ptr = nullptr) : ptr(_ptr) {}
	dynamic_array_iterator(const dynamic_array_iterator&) = default;
	dynamic_array_iterator& operator=(const dynamic_array_iterator&) = default;
	dynamic_array_iterator& operator=(const Ty* _ptr) { ptr = _ptr; return *this; }
	// ITERATOR MOVEMENT
	/**
	 * \brief Addition-assignment operator, increments the iterator by given `amount`.
	 *
	 * \param amount Displacement quantity by which to increment the iterator.
	 * \return `*this`.
	 */
	dynamic_array_iterator& operator+=(const std::ptrdiff_t& amount) { ptr += amount; return *this; }
	/**
	 * \brief Subtraction-assignment operator, decrements the iterator by given `amount`.
	 *
	 * \param amount Displacement quantity by which to decrement the iterator.
	 * \return `*this`.
	 */
	dynamic_array_iterator& operator-=(const std::ptrdiff_t& amount) { ptr -= amount; return *this; }
	/**
	 * \brief Addition operator, increments a copy of the iterator by a given `amount` and returns the copy.
	 *
	 * \param amount Displacement quantity by which to increment the copied iterator.
	 * \return A copy of the iterator incremented by `amount`.
	 */
	dynamic_array_iterator operator+(const std::ptrdiff_t& amount) {
		auto tmp_ptr = ptr; // copy initial ptr value locally
		ptr += amount; // increment ptr by amount
		auto tmp(*this); // copy incremented iterator
		ptr = tmp_ptr; // reset ptr of this to initial value
		return tmp;
	}
	/**
	 * \brief Subtraction operator, decrements a copy of the iterator by a given `amount` and returns the copy.
	 *
	 * \param amount Displacement quantity by which to decrement the copied iterator.
 	 * \return A copy of the iterator decremented by `amount`.
	 */
	dynamic_array_iterator operator-(const std::ptrdiff_t& amount) {
		auto tmp_ptr = ptr; // copy initial ptr value locally
		ptr -= amount; // decrementt ptr by amount
		auto tmp(*this); // copy decremented iterator
		ptr = tmp_ptr; // reset ptr of this to initial value
		return tmp;
	}
	/**
	 * \brief Distance operator, gets the distance between the iterator and `_other` iterator.
	 *
	 * \param _other Iterator to measure distance against.
	 * \return `std::distance` between this iterator and `_other`.
	 */
	std::ptrdiff_t operator-(const dynamic_array_iterator& _other) { return _other.ptr - ptr; }
	/**
	 * \brief Pre-increment operator, increments the iterator by a single unit.
	 *
	 * \return `*this`.
	 */
	dynamic_array_iterator& operator++() { ptr++; return *this; }
	/**
	 * \brief Post-increment operator, increments the iterator by a single unit and returns a copy of the
	 *        iterator before incrementing.
	 *
	 * \return A copy of the iterator before increment.
	 */
	dynamic_array_iterator operator++(int) {
		dynamic_array_iterator<Ty> tmp(*this);
		operator++();
		return tmp;
	}
	/**
	 * \brief Pre-decrement operator, decrements the iterator by a single unit.
	 *
	 * \return `*this`.
	 */
	dynamic_array_iterator& operator--() { ptr--; return *this; }
	/**
	 * \brief Post-decrement operator, decrements the iterator by a single unit and returns a copy of the
	 *        iterator before decrementing.
	 *
	 * \return A copy of the iterator before decrement.
	 */
	dynamic_array_iterator operator--(int) {
		dynamic_array_iterator<Ty> tmp(*this);
		operator--();
		return tmp;
	}
	// ITERATOR ACCESS
	/**
	 * \brief Dereferences the iterator, providing a mutable reference to the underlying data.
	 *
	 * \return Reference to underlying data corresponding to iterator.
	 */
	Ty& operator*() { return *ptr; }
	/**
	 * \brief Dereferences the iterator, providing an immutable (const) reference to the underlying data.
	 *
	 * \return Constant reference to the underlying data corresponding to iterator.
	 */
	const Ty& operator*() const { return *ptr; }
	/**
	 * \brief Accesses a mutable pointer to the underlying data.
	 *
	 * \return Pointer to underlying data corresponding to iterator.
	 */
	Ty* operator->() { return ptr; }
	/**
	 * \brief Accesses an immutable (const) pointer to the underlying data.
	 *
	 * \return Constant pointer to the underlying data corresponding to iterator.
	 */
	const Ty* operator->() const { return ptr; }
	// ITERATOR COMPARISON
	bool operator==(const dynamic_array_iterator& _other) const { return ptr == _other.ptr; }
	bool operator!=(const dynamic_array_iterator& _other) const { return ptr != _other.ptr; }
	bool operator<(const dynamic_array_iterator& _other) const { return ptr < _other.ptr; }
	bool operator>(const dynamic_array_iterator& _other) const { return ptr > _other.ptr; }
	bool operator<=(const dynamic_array_iterator& _other) const { return ptr <= _other.ptr; }
	bool operator>=(const dynamic_array_iterator& _other) const { return ptr >= _other.ptr; }
private:
	Ty* ptr;
};

/**
 * \class dynamic_array
 *
 * \brief A container storing an array in contiguous storage with methods to expand and contract
 *        the storage as necessary. Almost identical to `std::vector` in semantics, this class is
 *        just "practise" for implementing a vector style container - prefer to use actual `std::vector`.
 *
 * \tparam The type of the stored elements.
 */
template<typename Ty>
class dynamic_array {
public:
	// PUBLIC API TYPE DEFINITIONS
	typedef Ty value_type;
	typedef Ty& reference;
	typedef const Ty& const_reference;
	typedef Ty* pointer;
	typedef const Ty* const_pointer;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef dynamic_array_iterator<Ty> iterator;
	typedef dynamic_array_iterator<const Ty> const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	// CONSTRUCTION/ASSIGNMENT
	/**
	 * \brief Constructs an empty container, default constructor.
	 */
	dynamic_array()
		: arr_capacity(0), arr_size(0) {
		allocate(0);
	}
	/**
	 * \brief Constructs the container with `count` default-inserted elements.
	 *
	 * \param count Number of default-inserted `Ty` elements to initialise the container with.
	 */
	dynamic_array(size_type count)
		: arr_capacity(count), arr_size(count) {
		allocate(count);
	}
	/**
	  * \brief Constructs the container with `count` copies of `_val`.
	  *
	  * \param count Number of copies of `_val` to insert.
	  * \param _val Value to initialise elements with.
	 */
	dynamic_array(size_type count, const value_type& _val)
		: arr_capacity(count), arr_size(count) {
		allocate(count);
		fill(_val);
	}
	/**
	 * \brief Copy-constructor. Constructs the container with a copy of the
	 *        contents of `_other`.
	 *
	 * \param _other Container to use as data source.
	 */
	dynamic_array(const dynamic_array& _other)
		: arr_capacity(_other.arr_capacity), arr_size(_other.arr_size) {
		allocate(arr_capacity);
		std::copy(_other.arr, _other.arr + _other.arr_size, arr);
	}
	/**
	 * \brief Move-constructor. Constructs the container with the contents
	 *        of `_other` using move-semantics.
	 */
	dynamic_array(dynamic_array&& _other)
		: arr_capacity(std::move(_other.arr_capacity)), arr_size(std::move(_other.arr_size)) {
		allocate(arr_capacity);
		std::move(_other.arr, _other.arr + arr_size, arr);
	}
	dynamic_array(std::initializer_list<value_type> ilist)
		: arr_capacity(ilist.size()), arr_size(ilist.size()) {
		allocate(arr_capacity);
		std::move(ilist.begin(), ilist.end(), arr);
	}
	/**
	 * \brief Destructor. Calls the destructors of all elements in the container and deallocates
	 *        all memory used by the container.
	 */
	~dynamic_array() { destroy(); }
	/**
	 * \brief Assignment operator. Replaces the contents of the container with the contents of
	 *        `_other` either by copying or by move-semantics depending upon caller pass options.
	 *
	 * \param _other Container to use as data source.
	 * \return `*this`.
	 */
	dynamic_array& operator=(dynamic_array _other) {
		swap(*this, _other);
		return *this;
	}
	// CAPACITY
	/**
	 * \brief Checks if the container is empty.
	 *
	 * \return `true` if the container has no elements, `false` otherwise.
	 */
	bool empty() const noexcept { return arr_size; }
	/**
	 * \brief Returns the size of the container.
	 *
	 * \return Number of elements the container currently holds.
	 */
	size_type size() const noexcept { return arr_size; }
	/**
	 * \brief Returns the amount of elements the container currently has allocated space for.
	 *
	 * \return Current allocated storage size.
	 */
	size_type capacity() const noexcept { return arr_capacity; }
	/**
	 * \brief Reserves a given number of elements `new_cap` in allocated storage, altering
	 *        the `capacity()` of the container to `new_cap`. Only performs this reallocation
	 *        if `new_cap > capacity()` otherwise this method does nothing.
	 *
	 * \param new_cap New number of elements to allocate storage for.
	 */
	void reserve(size_type new_cap) { if (new_cap > arr_capacity) reallocate(new_cap); }
	// ELEMENT ACCESS
	/**
	 * \brief Returns a `reference` to the element at specified index `n` of container. Undefined
	 *        behaviour occurs if `!(n < size())`.
	 *
	 * \return `reference` to element at index `n`.
	 */
	reference operator[](size_type n) { return arr[n]; }
	/**
	 * \brief Returns a `const_reference` to the element at specified index `n` of container. Undefined
	 *        behaviour occurs if `!(n < size())`.
	 *
	 * \return `const_reference` to element at index `n`.
	 */
	const_reference operator[](size_type n) const { return arr[n]; }
	/**
	 * \brief Returns a `reference` to the element at specified index `n` of container.
	 *
	 * \return `reference` to element at index `n`.
	 * \throw Throws `std::out_of_range` exception if `!(n < size())`.
	 */
	reference at(size_type n) {
		if (!(n < arr_size)) throw std::out_of_range("dynamic_array index out of bounds.");
		return arr[n];
	}
	/**
	 * \brief Returns a `const_reference` to the element at specified index `n` of container.
	 *
	 * \return `const_reference` to element at index `n`.
	 * \throw Throws `std::out_of_range` exception if `!(n < size())`.
	 */
	const_reference at(size_type n) const {
		if (!(n < arr_size)) throw std::out_of_range("dynamic_array index out of bounds.");
		return arr[n];
	}
	pointer data() { return &arr[0]; }
	const_pointer data() const { return &arr[0]; }
	// MODIFIERS
	/**
	 * \brief Removes all elements from the container, leaves `capacity()` unchanged.
	 */
	void clear() {
		for (size_type i = 0; i < arr_size; ++i) {
			(&arr[i])->~value_type();
		}
		arr_size = 0;
	}
	iterator erase(const_iterator pos) {
		size_type pos_index = std::distance(cbegin(), pos);
		while (pos_index < arr_size - 1) {
			std::swap(arr[pos_index], arr[pos_index + 1]);
			++pos_index;
		}
		pop_back();
		return iterator(&arr[std::distance(cbegin(), pos)]);
	}
	iterator insert(const_iterator pos, const value_type& _val) {
		size_type pos_index = std::distance(cbegin(), pos);
		size_type iter_index = arr_size;
		push_back(_val);
		while (iter_index > pos_index) {
			std::swap(arr[iter_index], arr[iter_index - 1]);
			--iter_index;
		}
		return iterator(&arr[pos_index]);
	}
	iterator insert(const_iterator pos, value_type&& _val) {
		size_type pos_index = std::distance(cbegin(), pos);
		size_type iter_index = arr_size;
		push_back(std::move(_val));
		while (iter_index > pos_index) {
			std::swap(arr[iter_index], arr[iter_index - 1]);
			--iter_index;
		}
		return iterator(&arr[pos_index]);
	}
	iterator insert(const_iterator pos, size_type count, const value_type& _val) {
		while (count) {
			insert(pos, _val);
			--count;
		}
		return iterator(&arr[std::distance(cbegin(), pos)]);
	}
	template<class InputIt>
	iterator insert(const_iterator pos, InputIt first, InputIt last) {
		while (first != last) {
			insert(pos, *(first++));
		}
		return iterator(&arr[std::distance(cbegin(), pos)]);
	}
	/**
	 * \brief Appends the given element `_val` to the end of the container via
	 *        copy-initialisation.
	 *
	 * \param _val The value of the element to append.
	 */
	void push_back(const value_type& _val) {
		// if size of container has reached capacity perform
		// reallocation to larger storage 
		if (arr_size == arr_capacity) reallocate((arr_capacity != 0) ? arr_capacity * 2 : 8);
		// use placement-new to push back _val
		pointer val = new (arr + arr_size) value_type(_val);
		++arr_size;
	}
	/**
	 * \brief Appends the given element `_val` to the end of the container via
	 *        move-initialisation.
	 *
	 * \param _val The value of the element to append.
	 */
	void push_back(value_type&& _val) {
		// if size of container has reached capacity perform
		// reallocation to larger storage 
		if (arr_size == arr_capacity) reallocate((arr_capacity != 0) ? arr_capacity * 2 : 8);
		// use placement-new to push back _val
		pointer val = new (arr + arr_size) value_type(std::move(_val));
		++arr_size;
	}
	/**
	 * \brief Removes the last element of the container. Undefined behaviour occurs if
	 *        the container is empty.
	 */
	void pop_back() {
		(&arr[arr_size - 1])->~value_type();	// destruct last element
		--arr_size;
	}
	/**
	 * \brief Resizes the container to contain `count` elements.
	 *
	 * If `count > size()` then `count - size()` default-inserted instances are
	 * pushed into the container. Else if `count < size()` then only the first
	 * `count` elements will remain in the container.
	 *
	 * \param count New size of the container.
	 */
	void resize(size_type count) {
		if (count == arr_size) return;
		size_type tmp_size_cpy = arr_size;
		if (count > arr_size) { // expand container
			reallocate(count); // reallocate storage to count elements
							   // push back count - arr_size default-constructed values
			for (size_type i = 0; i < count - tmp_size_cpy; ++i)
				push_back(std::move(value_type()));
		}
		else if (count < arr_size) { // contract container
									 // pop back size() - count elements
			for (size_type i = 0; i < tmp_size_cpy - count; ++i)
				pop_back();
		}
	}
	/**
	 * \brief Resizes the container to contain `count` elements with any extra elements
	 *        added taking the value `_val`.
	 *
	 * If `count > size()` then `count - size()` instances of `_val` are pushed
	 * into the container. Else if `count < size()` then only the first `count`
	 * elements will remain in the container.
	 *
	 * \param count New size of the container.
	 * \param _val Value to assign to any new elements in the container.
	 */
	void resize(size_type count, const value_type& _val) {
		if (count == arr_size) return;
		size_type tmp_size_cpy = arr_size;
		if (count > arr_size) { // expand container
			reallocate(count); // realloc storage to count elements
							   // push back count - arr_size copies of _val
			for (size_type i = 0; i < count - tmp_size_cpy; ++i)
				push_back(_val);
		}
		else if (count < arr_size) { // contract container
									 // pop back size() - count elements
			for (size_type i = 0; i < tmp_size_cpy - count; ++i)
				pop_back();
		}
	}
	/**
	 * \brief Exchanges the contents of the container with those of `_other`.
	 *
	 * \_other Container to exchange with.
	 */
	void swap(dynamic_array& _other) {
		std::swap(arr, _other.arr);
		std::swap(arr_capacity, _other.arr_capacity);
		std::swap(arr_size, _other.arr_size);
	}
	static void swap(dynamic_array& lhs, dynamic_array& rhs) { lhs.swap(rhs); }
	// ITERATORS
	iterator begin() const noexcept { return iterator(&arr[0]); }
	iterator end() const noexcept { return iterator(&arr[arr_size]); }
	const_iterator cbegin() const noexcept { return const_iterator(&arr[0]); }
	const_iterator cend() const noexcept { return const_iterator(&arr[arr_size]); }
	reverse_iterator rbegin() const noexcept { return reverse_iterator(&arr[0]); }
	reverse_iterator rend() const noexcept { return reverse_iterator(&arr[arr_size]); }
	const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(&arr[0]); }
	const_reverse_iterator crend() const noexcept { return const_reverse_iterator(&arr[arr_size]); }
private:
	value_type* arr;
	size_type arr_capacity;
	size_type arr_size;
	/**
	 * \brief Dynamically allocate `n` elements of `arr`.
	 *
	 * \param n Number of elements to allocate.
	 */
	void allocate(size_type n) {
		arr = new value_type[n];
	}
	/**
	 * \brief Reallocate the array to a new capacity, `new_cap`.
	 *
	 * \param new_cap Number of elements to reallocate to.
	 */
	void reallocate(size_type new_cap) {
		value_type* tmp = new value_type[new_cap];	// allocate temp array
		size_type tmp_rows = (new_cap > arr_capacity) ? arr_capacity : new_cap;
		// move all elements from arr to temp array
		for (size_type i = 0; i < tmp_rows; ++i)
			tmp[i] = std::move(arr[i]);
		delete[] arr; // deallocate arr
		arr = tmp; // reassign arr to tmp
		arr_capacity = new_cap;
	}
	/**
	 * \brief Deallocates dynamically allocated array storage.
	 */
	void destroy() {
		//clear();
		delete[] arr;
	}
	/**
	 * \brief Fills array with given value `_val`.
	 *
	 * \param _val Value to fill container with.
	 */
	void fill(const value_type& _val) {
		for (size_type i = 0; i < arr_size; ++i)
			arr[i] = _val;
	}
};

#endif // !DYNAMIC_ARRAY_H
