#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>

template<typename T> class dynamic_array_const_iterator;

/**
* \class dynamic_array_iterator
*
* \brief A bidirectional iterator used for the template container `dynamic_array`. Uses the
*        `std::bidirectional_iterator_tag` and provides bidirectional iterator semantics -
*        giving random access, forward and backward iterator support.
*
* \tparam Ty The type of the stored elements.
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
	dynamic_array_iterator(const dynamic_array_const_iterator<Ty>& citer)
		: ptr() {
	}
	dynamic_array_iterator& operator=(const dynamic_array_iterator&) = default;
	dynamic_array_iterator& operator=(const Ty* _ptr) { ptr = _ptr; return *this; }
	dynamic_array_iterator& operator=(const dynamic_array_const_iterator<Ty>& citer) { ptr = citer.get_ptr(); return *this; }
	// ITERATOR MOVEMENT
	dynamic_array_iterator& operator+=(const std::ptrdiff_t& amount) { ptr += amount; return *this; }
	dynamic_array_iterator& operator-=(const std::ptrdiff_t& amount) { ptr -= amount; return *this; }
	dynamic_array_iterator operator+(const std::ptrdiff_t& amount) {
		auto tmp_ptr = ptr; // copy initial ptr value locally
		ptr += amount; // increment ptr by amount
		auto tmp(*this); // copy incremented iterator
		ptr = tmp_ptr; // reset ptr of this to initial value
		return tmp;
	}
	dynamic_array_iterator operator-(const std::ptrdiff_t& amount) {
		auto tmp_ptr = ptr; // copy initial ptr value locally
		ptr -= amount; // decrementt ptr by amount
		auto tmp(*this); // copy decremented iterator
		ptr = tmp_ptr; // reset ptr of this to initial value
		return tmp;
	}
	std::ptrdiff_t operator-(const dynamic_array_iterator& _other) { return _other.ptr - ptr; }
	dynamic_array_iterator& operator++() { ptr++; return *this; }
	dynamic_array_iterator operator++(int) {
		dynamic_array_iterator<Ty> tmp(*this);
		operator++();
		return tmp;
	}
	dynamic_array_iterator& operator--() { ptr--; return *this; }
	dynamic_array_iterator operator--(int) {
		dynamic_array_iterator<Ty> tmp(*this);
		operator--();
		return tmp;
	}
	// ITERATOR ACCESS
	Ty& operator*() { return *ptr; }
	const Ty& operator*() const { return *ptr; }
	Ty* operator->() { return ptr; }
	const Ty* operator->() const { return ptr; }
	Ty* get_ptr() { return ptr; }
	const Ty* get_ptr() const { return ptr; }
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

template<typename Ty>
class dynamic_array_const_iterator
	: public std::iterator<
	std::bidirectional_iterator_tag,
	Ty
	> {
public:
	// CONSTRUCTION/ASSIGNMENT
	dynamic_array_const_iterator(const Ty* _ptr = nullptr) : ptr(_ptr) {}
	dynamic_array_const_iterator(const dynamic_array_const_iterator&) = default;
	dynamic_array_const_iterator(const dynamic_array_iterator<Ty>& iter)
		: ptr(iter.get_ptr()) {
	}
	dynamic_array_const_iterator& operator=(const dynamic_array_const_iterator&) = default;
	dynamic_array_const_iterator& operator=(const Ty* _ptr) { ptr = _ptr; return  *this; }
	dynamic_array_const_iterator& operator=(const dynamic_array_iterator<Ty>& iter) {
		ptr = iter.get_ptr();
		return *this;
	}
	// ITERATOR MOVEMENT
	dynamic_array_const_iterator& operator+=(const std::ptrdiff_t& amount) { ptr += amount; return *this; }
	dynamic_array_const_iterator& operator-=(const std::ptrdiff_t& amount) { ptr -= amount; return *this; }
	dynamic_array_const_iterator operator+(const std::ptrdiff_t& amount) {
		auto tmp_ptr = ptr;
		ptr += amount;
		auto tmp(*this);
		ptr = tmp_ptr;
		return tmp;
	}
	dynamic_array_const_iterator operator-(const std::ptrdiff_t& amount) {
		auto tmp_ptr = ptr;
		ptr -= amount;
		auto tmp(*this);
		ptr = tmp_ptr;
		return tmp;
	}
	std::ptrdiff_t operator-(const dynamic_array_const_iterator& _other) { return _other.ptr - ptr; }
	dynamic_array_const_iterator& operator++() { ptr++; return *this; }
	dynamic_array_const_iterator operator++(int) { dynamic_array_const_iterator<Ty> tmp(*this); operator++(); return tmp; }
	dynamic_array_const_iterator& operator--() { ptr--; return *this; }
	dynamic_array_const_iterator operator--(int) { dynamic_array_const_iterator<Ty> tmp(*this); operator--(); return tmp; }
	// ITERATOR ACCESS
	const Ty& operator*() const { return *ptr; }
	const Ty* operator->() const { return ptr; }
	const Ty* get_ptr() const { return ptr; }
	// ITERATOR COMPARISON
	bool operator==(const dynamic_array_const_iterator& _other) const { return ptr == _other.ptr; }
	bool operator!=(const dynamic_array_const_iterator& _other) const { return ptr != _other.ptr; }
	bool operator<(const dynamic_array_const_iterator& _other) const { return ptr < _other.ptr; }
	bool operator>(const dynamic_array_const_iterator& _other) const { return ptr > _other.ptr; }
	bool operator<=(const dynamic_array_const_iterator& _other) const { return ptr <= _other.ptr; }
	bool operator>=(const dynamic_array_const_iterator& _other) const { return ptr >= _other.ptr; }
private:
	const Ty* ptr;
};

// ITERATOR/CONST_ITERATOR COMPARISONS
template<typename Ty>
bool operator==(const dynamic_array_iterator<Ty>& iter, const dynamic_array_const_iterator<Ty>& citer) {
	return iter.get_ptr() == citer.get_ptr();
}
template<typename Ty>
bool operator!=(const dynamic_array_iterator<Ty>& iter, const dynamic_array_const_iterator<Ty>& citer) {
	return !(iter == citer);
}
template<typename Ty>
bool operator<(const dynamic_array_iterator<Ty>& iter, const dynamic_array_const_iterator<Ty>& citer) {
	return iter.get_ptr() < citer.get_ptr();
}
template<typename Ty>
bool operator>(const dynamic_array_iterator<Ty>& iter, const dynamic_array_const_iterator<Ty>& citer) {
	return iter.get_ptr() > citer.get_ptr();
}
template<typename Ty>
bool operator<=(const dynamic_array_iterator<Ty>& iter, const dynamic_array_const_iterator<Ty>& citer) {
	return iter.get_ptr() <= citer.get_ptr();
}
template<typename Ty>
bool operator>=(const dynamic_array_iterator<Ty>& iter, const dynamic_array_const_iterator<Ty>& citer) {
	return iter.get_ptr() >= citer.get_ptr();
}

/**
* \class dynamic_array
*
* \brief A container storing an array in contiguous storage with methods to expand and contract
*        the storage as necessary.
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
	typedef dynamic_array_const_iterator<Ty> const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	// CONSTRUCTION/ASSIGNMENT
	dynamic_array()
		: arr_capacity(0), arr_size(0) {
		allocate(0);
	}
	dynamic_array(size_type count)
		: arr_capacity(count), arr_size(count) {
		allocate(count);
	}
	dynamic_array(size_type count, const value_type& _val)
		: arr_capacity(count), arr_size(count) {
		allocate(count);
		fill(_val);
	}
	dynamic_array(const dynamic_array& _other)
		: arr_capacity(_other.arr_capacity), arr_size(_other.arr_size) {
		allocate(arr_capacity);
		std::copy(_other.arr, _other.arr + _other.arr_size, arr);
	}
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
	~dynamic_array() { destroy(); }
	dynamic_array& operator=(dynamic_array _other) {
		swap(*this, _other);	// copy-and-swap
		return *this;
	}
	// CAPACITY
	bool empty() const noexcept { return arr_size; }
	size_type size() const noexcept { return arr_size; }
	size_type capacity() const noexcept { return arr_capacity; }
	void reserve(size_type new_cap) { if (new_cap > arr_capacity) reallocate(new_cap); }
	void shrink_to_fit() { if (arr_size < arr_capacity) reallocate(arr_size); }
	// ELEMENT ACCESS
	reference operator[](size_type n) { return arr[n]; }
	const_reference operator[](size_type n) const { return arr[n]; }
	reference at(size_type n) {	// bounds-checking mutable access
		if (!(n < arr_size)) throw std::out_of_range("dynamic_array index out of bounds.");
		return arr[n];
	}
	const_reference at(size_type n) const { // bounds-checking immutable
		if (!(n < arr_size)) throw std::out_of_range("dynamic_array index out of bounds.");
		return arr[n];
	}
	pointer data() { return &arr[0]; }
	const_pointer data() const { return &arr[0]; }
	// MODIFIERS
	void clear() {	// remove all container elements, leaving capacity unchanged
		for (size_type i = 0; i < arr_size; ++i) { // destruct each element
			(&arr[i])->~value_type();
		}
		arr_size = 0;
	}
	iterator erase(const_iterator pos) {	// erase element at position pos
		size_type pos_index = std::distance(cbegin(), pos);
		while (pos_index < arr_size - 1) {
			std::swap(arr[pos_index], arr[pos_index + 1]);
			++pos_index;
		}
		pop_back();
		return iterator(&arr[std::distance(cbegin(), pos)]);
	}
	iterator erase(const_iterator first, const_iterator last) {	// erase range [first, last) from container
		while (first != last) {
			first = erase(first++);
		}
		return first;
	}
	iterator insert(const_iterator pos, const value_type& _val) {	// insert _val to position pos
		size_type pos_index = std::distance(cbegin(), pos);
		size_type iter_index = arr_size;
		push_back(_val);
		while (iter_index > pos_index) {
			std::swap(arr[iter_index], arr[iter_index - 1]);
			--iter_index;
		}
		return iterator(&arr[pos_index]);
	}
	iterator insert(const_iterator pos, value_type&& _val) {	// insert _val via move-semantics to position pos
		size_type pos_index = std::distance(cbegin(), pos);
		size_type iter_index = arr_size;
		push_back(std::move(_val));
		while (iter_index > pos_index) {
			std::swap(arr[iter_index], arr[iter_index - 1]);
			--iter_index;
		}
		return iterator(&arr[pos_index]);
	}
	iterator insert(const_iterator pos, size_type count, const value_type& _val) {	// insert count copies of _val to position pos
		while (count) {
			insert(pos, _val);
			--count;
		}
		return iterator(&arr[std::distance(cbegin(), pos)]);
	}
	template<class InputIt>
	iterator insert(const_iterator pos, InputIt first, InputIt last) {	// insert range [first, last) to position pos
		while (first != last) {
			insert(pos, *(first++));
		}
		return iterator(&arr[std::distance(cbegin(), pos)]);
	}
	void push_back(const value_type& _val) {	// push element to back of container
		// if size of container has reached capacity perform
		// reallocation to larger storage 
		if (arr_size == arr_capacity) reallocate((arr_capacity != 0) ? arr_capacity * 2 : 8);
		// use placement-new to push back _val
		pointer val = new (arr + arr_size) value_type(_val);
		++arr_size;
	}
	void push_back(value_type&& _val) {	// push element to back of container via move-semantics
		// if size of container has reached capacity perform
		// reallocation to larger storage 
		if (arr_size == arr_capacity) reallocate((arr_capacity != 0) ? arr_capacity * 2 : 8);
		// use placement-new to push back _val
		pointer val = new (arr + arr_size) value_type(std::move(_val));
		++arr_size;
	}
	void pop_back() {
		(&arr[arr_size - 1])->~value_type();	// destruct last element
		--arr_size;
	}
	void resize(size_type count) {	// resize container to contain count elements
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
	void resize(size_type count, const value_type& _val) {	// resize with any extra elements taking value _val
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
	void swap(dynamic_array& _other) {	// exchange container with contents of _other
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
	void allocate(size_type n) {	// allocate array storage
		arr = new value_type[n];
	}
	void reallocate(size_type new_cap) {	// reallocate array storage
		value_type* tmp = new value_type[new_cap];	// allocate temp array
		size_type tmp_rows = (new_cap > arr_capacity) ? arr_capacity : new_cap;
		// move all elements from arr to temp array
		for (size_type i = 0; i < tmp_rows; ++i)
			tmp[i] = std::move(arr[i]);
		delete[] arr; // deallocate arr
		arr = tmp; // reassign arr to tmp
		arr_capacity = new_cap;
	}
	void destroy() {	// deallocate array storage
		delete[] arr;
	}
	void fill(const value_type& _val) {	// fill array with _val
		for (size_type i = 0; i < arr_size; ++i)
			arr[i] = _val;
	}
};

#endif // !DYNAMIC_ARRAY_H