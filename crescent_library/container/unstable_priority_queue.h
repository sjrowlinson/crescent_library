#ifndef UNSTABLE_PRIORITY_QUEUE_H
#define UNSTABLE_PRIORITY_QUEUE_H
#include <algorithm>
#include <ostream>
#include <set>
#include <vector>
#include <utility>

namespace crsc {
	/**
	 * \class unstable_priority_queue
	 *
	 * \brief A container adaptor that provides constant time lookup of the largest (by default) element at the expense
	 *        of logarithmic insertion and extraction. Unlike `crsc::priority_queue`, this container has no invariant
	 *        such that the heap ordering can be broken by the user; therefore `bubble_up`, `bubble_down` and `heapify`
	 *        methods are provided as part of the containers' public API to allow correct heap re-ordering to be called.
	 *
	 * The element priorities are compared by default using `std::less<Ty>` such that the largest element is always at
	 * the top of the heap, this comparator can be altered as a template argument to any other `Compare` type such as 
	 * `std::greater<Ty>` which would define minimum binary heap behaviour such that the smallest element is always on
	 * the top of the heap (providing constant lookup for the smallest stored item).
	 *
	 * \tparam Ty The type of the stored elements.
	 * \tparam Container The type of the underlying container to use to store the elements. The container must satisfy the
	 *         requirements of `SequenceContainer` and its iterators must satisfy the requirements of `RandomAccessIterator`
	 *         (see C++ Concepts). Additionally, it must provide the following functions with the usual semantics:
	 *         - `push_back()`
	 *         - `pop_back()`
	 *         - `emplace_back()`
	 * \tparam Priority A `Compare` type providing strict weak ordering, defaults to `std::less<Ty>`.
	 * \author Samuel Rowlinson
	 * \date August, 2016
	 */
	template<class Ty,
		class Container = std::vector<Ty>,
		class Priority = std::less<Ty>
	> class unstable_priority_queue {
	public:
		// PUBLIC API TYPE DEFINITIONS
		typedef Ty value_type;
		typedef Ty& reference;
		typedef const Ty& const_reference;
		typedef Ty* pointer;
		typedef const Ty* const_pointer;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef typename Container::iterator iterator;
		typedef typename Container::const_iterator const_iterator;
		typedef typename Container::reverse_iterator reverse_iterator;
		typedef typename Container::const_reverse_iterator const_reverse_iterator;
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
		unstable_priority_queue(const Priority& compare, const Container& container)
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
		explicit unstable_priority_queue(const Priority& compare = Priority(), Container&& container = Container())
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
		unstable_priority_queue(InputIt first, InputIt last, const Priority& compare, const Container& container)
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
		unstable_priority_queue(InputIt first, InputIt last, const Priority& compare = Priority(), Container&& container = Container())
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
		unstable_priority_queue(const unstable_priority_queue& _other) noexcept
			: heap_cntr(_other.heap_cntr), comp(_other.comp) {}
		/**
		 * \brief Constructs the container with the contents of `_other` using move-semantics.
		 *
		 * \param _other Container to use as data source to initialise this container with.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		unstable_priority_queue(unstable_priority_queue&& _other) noexcept
			: heap_cntr(std::move(_other.heap_cntr)), comp(std::move(_other.comp)) {}
		/**
		 * \brief Destructs the container. The destructors of the elements are called
		 *        used storage is deallocated.
		 *
		 * \complexity Linear in the size of the container.
		 * \exceptionsafety No-throw guarantee, implicitly `noexcept` specification.
		 */
		~unstable_priority_queue() {}
		/**
		 * \brief Copy-assignment operator, replaces the contents of the container with
		 *        the a copy of the contents of `_other`.
		 *
		 * \param _other Another `unstable_priority_queue` container to be used as data source.
		 * \return `*this`.
		 * \complexity Linear in the size of the `_other`.
		 */
		unstable_priority_queue& operator=(const unstable_priority_queue& _other) {
			if (this != &_other)
				unstable_priority_queue(_other).swap(*this);
			return *this;
		}
		/**
		 * \brief Move-assignment operator, replaces the contents of the container with
		 *        the contents of `_other` using move-semantics.
		 *
		 * \param _other Another `unstable_priority_queue` container to be used as data source.
		 * \return `*this`.
		 * \complexity Constant.
		 */
		unstable_priority_queue& operator=(unstable_priority_queue&& _other) {
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
		 * \brief Accesses a `const_reference` to the top element of the container without popping it.
		 *
		 * \return `const_reference` to top element of the container.
		 * \complexity Constant.
		 * \exceptionsafety If `!empty()` then no-throw guarantee, otherwise
		 *                  undefined behaviour.
		 */
		const_reference top() const {
			return heap_cntr[0];
		}
		/**
		 * \brief Accesses a `reference` to the top element of the container without popping it.
		 *
		 * \return `reference` to top element of the container.
		 * \complexity Constant.
		 * \exceptionsafety If `!empty()` then no-throw guarantee, otherwise 
		 *                  undefined behaviour.
		 */
		reference top() {
			return heap_cntr[0];
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
		void enqueue(const value_type& _val) {
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
		void enqueue(value_type&& _val) {
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
		void emplace(Args&&... args) {
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
			size_type heap_size = heap_cntr.size();
			if (!heap_size) return;	// empty container, do nothing
			// set top of heap to last element and pop last element
			heap_cntr[0] = heap_cntr[heap_size - 1];
			heap_cntr.pop_back();
			// bubble top of heap downwards to correct position
			bubble_down(0);
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
		iterator erase(const_iterator pos) {
			auto it = heap_cntr.erase(pos);
			bubble_down(std::distance(heap_cntr.begin(), it));
			return it;
		}
		iterator erase(const_iterator first, const_iterator last) {
			auto it = heap_cntr.erase(first, last);
			heapify();
			return it;
		}
		void swap(unstable_priority_queue& _other) {
			heap_cntr.swap(_other.heap_cntr);
			std::swap(comp, _other.comp);
		}
		static void swap(unstable_priority_queue& lhs, unstable_priority_queue& rhs) {
			lhs.swap(rhs);
		}
		// HEAP ORDERING
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
		// ITERATORS
		iterator begin() noexcept {
			return heap_cntr.begin();
		}
		const_iterator cbegin() const noexcept {
			return heap_cntr.cbegin();
		}
		iterator end() noexcept {
			return heap_cntr.end();
		}
		const_iterator cend() const noexcept {
			return heap_cntr.cend();
		}
		reverse_iterator rbegin() noexcept {
			return heap_cntr.rbegin();
		}
		const_reverse_iterator crbegin() const noexcept {
			return heap_cntr.crbegin();
		}
		reverse_iterator rend() noexcept {
			return heap_cntr.rend();
		}
		const_reverse_iterator crend() const noexcept {
			return heap_cntr.crend();
		}
	private:
		Container heap_cntr;
		Priority comp;
	};
}

#endif // !UNSTABLE_PRIORITY_QUEUE_H
