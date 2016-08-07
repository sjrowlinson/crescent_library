#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H
#include "sfinae_operators.h"
#include <algorithm>
#include <ostream>
#include <set>
#include <vector>
#include <utility>

namespace crsc {
	/**
	 * \class priority_queue
	 *
	 * \brief A container adaptor (wrapping a `std::vector` as the heap storage) that provides constant time lookup
	 *        of the largest (by default) element, at the expense of logarithmic insertion and extraction.
	 *
	 * The element priorities are compared by default using `std::less<_Ty>` such that the largest element is always
	 * at the top of the heap, this comparator can be altered as a template argument to any other `Compare` type such
	 * as `std::greater<_Ty>` which would define minimum binary heap behaviour such that the smallest element is
	 * always on the top of the heap (providing constant lookup for the smallest item).
	 *
	 * Unlike `std::priority_queue` this container only allows the wrapping of a `std::vector` as the underlying
	 * heap container. However, this version of a priority queue provides methods to search for items in the container,
	 * alter item values, clear the container and iterate over the container with `const_iterator`s which the 
	 * STL priority queue does not provide. In addition, similarly to `std::priority_queue`, every method is guaranteed
	 * to preserve the class invariant such that the heap is not invalidated at any point between method calls.
	 *
	 * \tparam _Ty The type of the elements.
	 * \tparam _Cntr The type of the underlying container to use to store the elements. The container must satisfy the
	 *         requirements of `SequenceContainer` and its iterators must satisfy the requirements of `RandomAccessIterator`
	 *         (see C++ Concepts). Additionally, it must provide the following functions with the usual semantics:
	 *         - `push_back()`
	 *         - `pop_back()`
	 *         - `emplace_back()`
	 * \tparam _Pr A `Compare` type providing a strict weak ordering, defaults to `std::less<_Ty>`.
	 * \invariant The heap (whose ordering/behaviour is defined by the comparator `_Pr`) shall never be invalidated
	 *            between method calls, and if any exceptions are thrown by a method the heap shall never be left in
	 *            a state which would invalidate the heap.
	 * \author Samuel Rowlinson
	 * \date July, 2016
	 */
	template<typename _Ty,
		class _Cntr = std::vector<_Ty>,
		class _Pr = std::less<_Ty>
	> class priority_queue {
	public:
		// PUBLIC API TYPE DEFINITIONS
		typedef _Ty value_type;
		typedef _Ty& reference;
		typedef const _Ty& const_reference;
		typedef _Ty* pointer;
		typedef const _Ty* const_pointer;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef typename _Cntr::const_iterator const_iterator;
		typedef typename _Cntr::const_reverse_iterator const_reverse_iterator;
		// CONSTRUCTION/ASSIGNMENT
		/**
		 * \brief Copy-constructs the underlying container with the contents of `container`. Copy-constructs
		 *        the underlying comparison functor with the contents of `compare`.
		 *
		 * \param compare Comparator function object to initialise underlying comparison functor.
		 * \param container Container to be used as source to initialise the underlying container.
		 * \complexity Linear in the size of `container` (comparisons) plus linear in the size
		 *             of `container` (calls to constructor of `value_type`).
		 */
		priority_queue(const _Pr& compare, const _Cntr& container)
			: heap_cntr(container), comp(compare) { heapify(); }
		/**
		 * \brief Move-constructs the underlying container with `std::move(container)`. Copy-constructs
		 *        the underlying comparison functor with the contents of `compare`. This is also the
		 *        default constructor.
		 *
		 * \param compare Comparator function object to initialise underlying comparison functor.
		 * \param container Container to be used as source to initialise the underlying container.
		 * \complexity Linear in the size of `container` (comparisons).
		 * \exceptionsafety No-throw guarantee if `_Pr()` does not throw, otherwise dependent upon
		 *                  exception safety of `compare`.
		 */
		explicit priority_queue(const _Pr& compare = _Pr(), _Cntr&& container = _Cntr()) 
			: heap_cntr(std::move(container)), comp(compare) { heapify(); }
		/**
		 * \brief Copy-constructs the underlying container with the contents of `container` and the underlying
		 *        comparison functor with the contents of `compare` then inserts the contents of the range
		 *        `[first, last)` to the end of the underlying container.
		 *
		 * \param first Begininning of range to copy elements from.
		 * \param last End of range to copy elements from.
		 * \param compare Comparator function object to initialise underlying comparison functor.
		 * \param container Container to be used as source to initialise the underlying container.
		 */
		template<class InputIt>
		priority_queue(InputIt first, InputIt last, const _Pr& compare, const _Cntr& container)
			: heap_cntr(container), comp(compare) {
			heap_cntr.insert(heap_cntr.end(), first, last);
			heapify();
		}
		/**
		 * \brief Move-constructs the underlying container with `std::move(container)` and copy-constructs
		 *       the underlying comparison functor with the contents of `compare` then inserts the contents
		 *       of the range `[first, last)` to the end of the underlying container.
		 *
		 * \param first Beginning of range to copy elements from.
		 * \param last End of range to copy elements from.
		 * \param compare Comparator function object to initialise underlying comparison functor.
		 * \param container Container to be used as source to initialise the underlying container.
		 */
		template<class InputIt>
		priority_queue(InputIt first, InputIt last, const _Pr& compare = _Pr(), _Cntr&& container = _Cntr())
			: heap_cntr(std::move(container)), comp(compare) { 
			heap_cntr.insert(heap_cntr.end(), first, last);
			heapify();
		}
		/**
		 * \brief Constructs the container with a copy of the contents of `_other`.
		 *
		 * \param _other Container to use as data source to initialise this container with.
		 * \complexity Linear in the size of `_other`.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		priority_queue(const priority_queue& _other) noexcept
			: heap_cntr(_other.heap_cntr), comp(_other.comp) {}
		/**
		 * \brief Constructs the container with the contents of `_other` using move-semantics.
		 *
		 * \param _other Container to use as data source to initialise this container with.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		priority_queue(priority_queue&& _other) noexcept
			: heap_cntr(std::move(_other.heap_cntr)), comp(std::move(_other.comp)) {}
		/**
		 * \brief Destructs the container. The destructors of the elements are called
		 *        used storage is deallocated.
		 *
		 * \complexity Linear in the size of the container.
		 * \exceptionsafety No-throw guarantee, implicitly `noexcept` specification.
		 */
		~priority_queue() {}
		/**
		 * \brief Copy-assignment operator, replaces the contents of the container with
		 *        the a copy of the contents of `_other`.
		 *
		 * \param _other Another `priority_queue` container to be used as data source.
		 * \return `*this`.
		 * \complexity Linear in the size of the `_other`.
		 */
		priority_queue& operator=(priority_queue _other) {
			if (this != &_other)
				swap(*this, _other); // invoke copy-and-swap idiom
			return *this;
		}
		/**
		 * \brief Move-assignment operator, replaces the contents of the container with
		 *        the contents of `_other` using move-semantics.
		 *
		 * \param _other Another `priority_queue` container to be used as data source.
		 * \return `*this`.
		 * \complexity Constant.
		 */
		priority_queue& operator=(priority_queue&& _other) {
			if (this != &_other)
				swap(*this, _other);
			return *this;
		}
		// CAPACITY
		/**
		 * \brief Checks whether the container is empty.
		 * 
		 * \return `true` if empty, `false` otherwise.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		bool empty() const noexcept {
			return heap_cntr.empty();
		}
		/**
		 * \brief Returns the number of elements in the container.
		 *
		 * \return Number of elements in the container.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		size_type size() const noexcept {
			return heap_cntr.size();
		}
		/**
		 * \brief Returns the maximum number of elements the container is able
		 *        to hold due to system or library implementation limitations.
		 *
		 * \return Maximum number of elements.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		size_type max_size() const noexcept {
			return heap_cntr.max_size();
		}
		// ELEMENT ACCESS
		/**
		 * \brief Accesses the top element of the container without popping it.
		 *
		 * \return const_reference to top element of the container.
		 * \complexity Constant.
		 * \exceptionsafety If `!empty()` then no-throw guarantee, otherwise 
		 *                  undefined behaviour.
		 */
		const_reference top() const {
			return heap_cntr[0];
		}
		/**
		 * \brief Finds the first instance of an element in the container.
		 *
		 * \param _val Value to search for in the container.
		 * \return const_iterator pointing to found element, `crsc::priority_queue::cend()` if not found.
		 * \complexity Linear in the size of the container.
		 * \exceptionsafety No-throw guarantee.
		 */
		const_iterator find(const value_type& _val) const {
			return std::find(heap_cntr.begin(), heap_cntr.end(), _val);
		}
		/**
		 * \brief Finds all instances of an element in the container.
		 *
		 * \param _val Value to search for in the container.
		 * \return `std::set<const_iterator>` containing `const_iterator`s to each item with value
		 *         `_val` in the container.
		 * \complexity Linear in the size of the container.
		 * \exceptionsafety Strong-guarantee, if an exception is thrown there are no changes
		 *                  in the container.
		 */
		std::set<const_iterator> find_all(const value_type& _val) const {
			std::set<const_iterator> it_set;
			for (auto it = heap_cntr.cbegin(); it < heap_cntr.cend(); ++it) {
				if (*it == _val) it_set.insert(it);
			}
			return it_set;
		}
		/**
		 * \brief Find the first instance of an element in the container based on
		 *        a unary predicate.
		 *
		 * \param _p Unary predicate which returns `true` for the required element.
		 * \return `const_iterator` pointing to found element, `crsc::priority_queue::cend()` if not found.
		 * \complexity Linear in the size of the container.
		 * \exceptionsafety No-throw guarantee.
		 */
		template<class UnaryPredicate>
		const_iterator find(UnaryPredicate _p) const {
			return std::find_if(heap_cntr.begin(), heap_cntr.end(), _p);
		}
		/**
		 * \brief Finds all instances of an element in the container based on
		 *        a unary predicate.
		 *
		 * \param _p Unary predicate which returns `true` for the required element.
		 * \return `std::vector<const_iterator>` containing `const_iterator`s to each item in the
		 *         container which satisfies the predicate `_p`.
		 * \complexity Linear in the size of the container.
		 * \exceptionsafety Strong-guarantee, if an exception is thrown there are no changes
		 *                  in the container.
     	 */
		template<class UnaryPredicate>
		std::set<const_iterator> find_all(UnaryPredicate _p) const {
			std::set<const_iterator> it_set;
			for (auto it = heap_cntr.cbegin(); it < heap_cntr.cend(); ++it) {
				if (_p(*it)) it_set.insert(it);
			}
			return it_set;
		}
		/**
		 * \brief Writes the contents of the container to a `std::ostream` in heap-order.
		 *
		 * \param _os Instance of `std::ostream` to write to.
		 * \param _delim Delimiter for separation of container elements in stream.
		 * \return Reference to modified `_os`.
		 * \complexity Linear in the size of the container.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value
				&& has_insertion_operator<_Uty>::value>
		> std::ostream& write_ordered(std::ostream& _os, char _delim = ' ') const noexcept {
			priority_queue<value_type, _Pr> tmp(*this);
			while (!tmp.empty()) {
				_os << tmp.top() << _delim;
				tmp.dequeue();
			}
			return _os;
		}
		/**
		 * \brief Writes the contents of the container to a `std::ostream` in non-heap-order,
		 *        the elements shall be written in the order that they are stored in the 
		 *        underlying contiguous memory block.
		 *
		 * \param _os Instance of `std::ostream` to write to.
		 * \param _delim Delimiter for separation of container elements in stream.
		 * \return Reference to modified `_os`.
		 * \complexity Linear in the size of the container.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<has_insertion_operator<_Uty>::value>
		> std::ostream& write(std::ostream& _os, char _delim = ' ') const noexcept {
			for (const auto& el : heap_cntr)
				_os << el << _delim;
			return _os;
		}
		// MODIFIERS
		/**
		 * \brief Pushes an item into the container and sorts it.
		 *
		 * \param _val Value to enqueue into the container.
		 * \complexity Logarithmic in the size of the container plus amortized
		 *             constant for `std::vector::push_back` operation.
		 * \exceptionsafety Strong-guarantee, if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> void enqueue(const value_type& _val) {
			heap_cntr.push_back(_val);
			bubble_up(heap_cntr.size() - 1);
		}
		/**
		 * \brief Pushes an item into the container via move-insertion and sorts it.
		 *
		 * \param _val rvalue reference to value to enqueue into the container.
		 * \complexity Logarithmic in the size of the container plus amortized
		 *             constant for `std::vector::push_back` operation.
		 * \exceptionsafety Strong-guarantee, if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_move_assignable<_Uty>::value>
		> void enqueue(value_type&& _val) {
			heap_cntr.push_back(std::move(_val));
			bubble_up(heap_cntr.size() - 1);
		}
		/**
		 * \brief Constructs element in-place and sorts the underlying container.
		 *
		 * \param _args Arguments to forward to the constructor of the element.
		 * \complexity Logarithmic in the size of the container plus complexity
		 *             of single construction of argument of `value_type`.
		 * \exceptionsafety Strong-guarantee, if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class... Args>
		void emplace(Args&&... _args) {
			heap_cntr.emplace_back(std::forward<Args>(_args)...);
			bubble_up(heap_cntr.size() - 1);
		}
		/**
		 * \brief Pops the top item from the container.
		 * 
		 * \complexity Logarithmic in the size of the container.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification - even
		 *                  if container is `empty()` in which case `dequeue()`
		 *                  does nothing.
		 */
		void dequeue() noexcept {
			pop_top();
		}
		/**
		 * \brief Clears all items from the container.
		 *
		 * \complexity Linear in the size of the container.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		void clear() noexcept {
			heap_cntr.clear();
		}
		/**
		 * \brief Alters the first instance of the specified value (first of pair param)
		 *        in the container to second of pair param by copy-assignment. If value
		 *        to find does not exist in the container, this method does nothing.
		 *
		 * \param _tgt_alt `std::pair` where first field is value to find and alter and
		 *        second field is the value to which to change this found value.
		 * \complexity Linear in the size of the container plus logarithmic
		 *             in the size of the container.
		 * \exceptionsafety Strong-guarantee, if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> void alter(const std::pair<value_type, value_type>& _tgt_alt) {
			auto it = std::find(heap_cntr.begin(), heap_cntr.end(), _tgt_alt.first);
			if (it != heap_cntr.end()) {
				*it = _tgt_alt.second;
				auto index = std::distance(heap_cntr.begin(), it); // index of changed element
				comp(_tgt_alt.first, _tgt_alt.second) ? bubble_up(index) : bubble_down(index);
			}
		}
		/**
		 * \brief Alters the element value at the specified iterator position in the container
		 *        to the value `_alter_to_val` by copy-assignment.
		 *
		 * \param _pos `const_iterator` to element in container to alter.
		 * \param _alter_to_val Value to assign to element to alter.
		 * \complexity Logarithmic in the size of the container.
		 * \exceptionsafety Strong-guarantee, if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> void alter(const_iterator _pos, const value_type& _alter_to_val) {
			auto index = std::distance(heap_cntr.cbegin(), _pos); // index of changed element
			bool b_up = comp(heap_cntr[index], _alter_to_val);
			heap_cntr[index] = _alter_to_val;
			b_up ? bubble_up(index) : bubble_down(index);
		}
		/**
		 * \brief Alters the element value at the specified iterator position in the container
		 *        to the value `_alter_to_val` by move-assignment.
		 *
		 * \param _pos `const_iterator` to element in container to alter.
		 * \param _alter_to_val rvalue reference to value to assign to element to alter.
		 * \complexity Logarithmic in the size of the container.
		 * \exceptionsafety Strong-guarantee, if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<std::is_move_assignable<_Uty>::value>
		> void alter(const_iterator _pos, value_type&& _alter_to_val) {
			auto index = std::distance(heap_cntr.cbegin(), _pos); // index of changed element
			bool b_up = comp(heap_cntr[index], _alter_to_val);
			heap_cntr[index] = std::move(_alter_to_val);
			b_up ? bubble_up(index) : bubble_down(index);
		}
		/**
		 * \brief Alters the first instance of an item in the container for
		 *        which the unary predicate `_p` is satisfied to `_alter_to_val`
		 *        by copy-assignment. If there are no such items in the container
		 *        which satisfy `_p` then this method does nothing.
		 *
		 * \param _alter_to_val Value to assign search item to.
		 * \param _p Unary predicate which returns `true` for the required element.
		 * \complexity Linear in the size of the container plus logarithmic
		 *             in the size of the container.
		 * \exceptionsafety Strong-guarantee, if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class UnaryPredicate,
			class _Uty = _Ty,
			class = std::enable_if_t<std::is_copy_assignable<_Uty>::value>
		> void alter(const value_type& _alter_to_val, UnaryPredicate _p) {
			auto it = std::find_if(heap_cntr.begin(), heap_cntr.end(), _p);
			if (it != heap_cntr.end()) {
				bool b_up = comp(*it, _alter_to_val);
				*it = _alter_to_val;
				auto index = std::distance(heap_cntr.begin(), it); // index of changed element
				b_up ? bubble_up(index) : bubble_down(index);
			}
		}
		/**
		 * \brief Alters the first instance of an item in the container for
		 *        which the unary predicate `_p` is satisfifed to `_alter_to_val`
		 *        by move-assignment. If there are no such items in the container
		 *        which satisfy `_p` then this method does nothing.
		 *
		 * \param _alter_to_val rvalue reference to value to assign search item to.
		 * \param _p Unary predicate which returns `true` for the required element.
		 * \complexity Linear in the size of the container plus logarithmic
		 *             in the size of the container.
		 * \exceptionsafety Strong-guarantee, if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class UnaryPredicate,
			class _Uty = _Ty,
			class = std::enable_if_t<std::is_move_assignable<_Uty>::value>
		> void alter(value_type&& _alter_to_val, UnaryPredicate _p) {
			auto it = std::find_if(heap_cntr.begin(), heap_cntr.end(), _p);
			if (it != heap_cntr.end()) {
				bool b_up = comp(*it, _alter_to_val);
				*it = std::move(_alter_to_val);
				auto index = std::distance(heap_cntr.begin(), it); // index of changed element
				b_up ? bubble_up(index) : bubble_down(index);
			}
		}
		/**
		 * \brief Alters all instances of the specified value (first of pair `_tgt_alter`) in the container
		 *        to second of pair `_tgt_alter` by copy-assignment. If value to find does not exist in
		 *        the container, this method does nothing.
		 *
		 * \param _tgt_alt `std::pair` where first field is value to find and alter and
		 *        second field is the value to which to change this found value.
		 * \complexity Linear in the size of the container plus the number of
		 *             occurrences of `_val_find` in the container multiplied by
		 *             logarithmic in the size of the container.
		 * \exceptionsafety Strong-guarantee, if an exception is thrown there are no changes
		 *                  in the container.
		 */
		void alter_all(const std::pair<value_type, value_type>& _tgt_alter) {
			for (auto it = heap_cntr.begin(); it < heap_cntr.end(); ++it) {
				if (*it == _tgt_alter.first) *it = _tgt_alter.second;
			}
			heapify();
		}
		/**
		 * \brief Alters all instances of items in the container for which the unary 
		 *        predicate `_p` is satisifed to `_alter_to_val` by copy-assignment. If
		 *        there are no such items in the container which satisfy `_p` then this
		 *        method does nothing.
		 *
		 * \param _alter_to_val Value to assign each `_p` satisfied item to.
		 * \param _p Unary predicate which returns `true` for the required elements.
		 * \complexity Linear in the size of the container plus the number of
		 *             items for which `_p` is satisfied in the container multiplied
		 *             by logarithmic in the size of the container.
		 * \exceptionsafety Strong-guarantee, if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<class UnaryPredicate>
		void alter_all(const value_type& _alter_to_val, UnaryPredicate _p) {
			for (auto it = heap_cntr.begin(); it < heap_cntr.end(); ++it) {
				if (_p(*it)) *it = _alter_to_val;
			}
			heapify();
		}
		/**
		 * \brief Erases the element at the given position in the container.
		 *
		 * \param pos `const_iterator` to element in container to erase.
		 * \warning UNTESTED - REQUIRES TESTING TO ENSURE INVARIANT IS NOT VIOLATED.
		 */
		void erase(const_iterator pos) {
			auto it = heap_cntr.erase(pos);
			bubble_down(std::distance(heap_cntr.begin(), it));
		}
		/**
		 * \brief Erases the first occurrence of a given value `_val_erase` in the container.
		 *
		 * \param _val_erase Value to erase.
		 * \warning UNTESTED - REQUIRES TESTING TO ENSURE INVARIANT IS NOT VIOLATED.
		 */
		void erase(const value_type& _val_erase) {
			auto it = heap_cntr.erase(find(_val_erase));
			bubble_down(std::distance(heap_cntr.begin(), it));
		}
		/**
		 * \brief Erases the elements in the range `[first, last)` from the container.
		 *
		 * \param first Beginning of range to erase.
		 * \param last End of range to erase.
		 * \warning UNTESTED - REQUIRES TESTING TO ENSURE INVARIANT IS NOT VIOLATED.
		 */
		void erase(const_iterator first, const_iterator last) {
			heap_cntr.erase(first, last);
			heapify();
		}
		/** 
		 * \brief Exchanges the contents of the container with those of `_other`. Does not
		 *        cause references and iterators to associate with the other container.
		 *
		 * \param _other `priority_queue` container to swap with.
		 * \complexity Constant.
		 */
		void swap(priority_queue& _other) {
			heap_cntr.swap(_other.heap_cntr);
			std::swap(comp, _other.comp);
		}
		/**
		 * \brief Exchanges the contents of two `crsc::priority_queue` containers. Does not
		 *        cause references and iterators to associate with the other containers.
		 *
		 * \param lhs First instance of `priority_queue`.
		 * \param rhs Second instance of `priority_queue`.
		 * \complexity Constant.
		 */
		static void swap(priority_queue& lhs, priority_queue& rhs) {
			lhs.swap(rhs);
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
		const_iterator cbegin() const noexcept {
			return heap_cntr.cbegin();
		}
		/**
		 * \brief Returns a const_iterator to the past-the-end element of the container.
		 *
		 * \return Constant iterator to the past-the-end element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_iterator cend() const noexcept {
			return heap_cntr.cend();
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
			return heap_cntr.crbegin();
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
			return heap_cntr.crend();
		}
	private:
		_Cntr heap_cntr;	// underlying heap container
		_Pr comp;	// comparator function-object, determines element priorities
		/**
		 * \brief Bubbles down the heap from a given vector index, performing
		 *        swaps based on comparator conditions.
		 *
		 * \param _pos Index to perform bubbling down from.
		 * \complexity Logarithmic in the size of the container.
		 * \exceptionsafety If `_pos < size()` then no-throw guarantee, otherwise
		 *                  undefined behaviour.
		 */
		void bubble_down(size_type _pos) {
			size_type heap_size = heap_cntr.size();
			// positions of left, right nodes relative to parent in heap_cntr
			size_type left_child = 2 * _pos + 1;
			size_type right_child = 2 * _pos + 2;
			// reached end of heap, exit
			if (left_child >= heap_size) return;
			size_type min_pos = _pos;
			// perform comparison between values of heap at _pos and left node pos
			// and set min_pos to position of left node if comparison yields true
			if (comp(heap_cntr[_pos], heap_cntr[left_child]))
				min_pos = left_child;
			// perform comparison between values of heap at min_pos and right node pos
			// and set min_pos to position of right node if comparision yields true
			if (right_child < heap_size && comp(heap_cntr[min_pos], heap_cntr[right_child]))
				min_pos = right_child;
			// if required, perform swap and bubble down from swapped min_pos
			if (min_pos != _pos) {
				std::swap(heap_cntr[_pos], heap_cntr[min_pos]);
				bubble_down(min_pos);
			}
		}
		/**
		 * \brief Bubbles up the heap from a given vector index, performing
		 *        swaps based on comparator conditions.
		 *
		 * \param _pos Index to perform bubbling up from.
		 * \complexity Logarithmic in the size of the container.
		 * \exceptionsafety If `_pos < size()` then no-throw guarantee, otherwise
		 *                  undefined behaviour.
		 */
		void bubble_up(size_type _pos) {
			// at top of heap, exit
			if (!_pos) return;
			// positon of parent node relative to a child node in heap_cntr
			size_type parent = (_pos - 1) / 2;
			// perform comparison between heap values at parent and _pos
			// and do swap and bubble up from parent if comparison is true
			if (comp(heap_cntr[parent], heap_cntr[_pos])) {
				std::swap(heap_cntr[_pos], heap_cntr[parent]);
				bubble_up(parent);
			}
		}
		/**
		 * \brief Removes the top item from the heap and bubbles down from new top.
		 *
		 * \complexity Logarithmic in the size of the container.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		void pop_top() noexcept {
			size_type heap_size = heap_cntr.size();
			if (!heap_size) return;
			// set top of heap to last heap element then remove last heap element
			std::swap(heap_cntr[0], heap_cntr[--heap_size]);
			heap_cntr.pop_back();
			// bubble down from top to get previously last heap element to correct position
			bubble_down(0);
		}
		/**
		 * \brief Performs heapification of entire binary heap, bubbling down from
		 *        each index such that the binary heap invariant is guaranteed.
		 *
		 * \complexity Linear in the size of the container multiplied by
		 *             logarithmic in the size of the container.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		void heapify() noexcept {
			for (int i = heap_cntr.size() - 1; i > -1; --i)
				bubble_down(i);
		}
	};
	/**
	 * \brief Stream insertion operator for `crsc::priority_queue` of copyable type.
	 *
	 * \param _os Instance of `std::ostream` to write to.
	 * \param _pq `crsc::priority_queue` container to write to stream.
	 * \return Modified reference to `_os`.
	 * \complexity Linear in the size of `_pq`.
	 * \exceptionsafety No-throw guarantee, `noexcept` specification.
	 */
	template<typename _Ty,
		class _Cntr,
		class _Pr,
		typename = std::enable_if_t<std::is_copy_assignable<_Ty>::value>
	> std::ostream& operator<<(std::ostream& _os, const priority_queue<_Ty, _Cntr, _Pr>& _pq) noexcept {
		return _pq.write_ordered(_os);
	}
}

#endif // !PRIORITY_QUEUE_H
