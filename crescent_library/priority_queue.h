#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H
#include <algorithm>
#include <ostream>
#include <stdexcept>
#include <vector>
#include <utility>

namespace crsc {

	template<typename _Ty,
		class _Pr = std::less<_Ty>
	> class priority_queue {
	public:
		typedef _Ty value_type;
		typedef _Ty& reference;
		typedef const _Ty& const_reference;
		typedef _Ty* pointer;
		typedef const _Ty* const_pointer;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef typename std::vector<_Ty>::const_iterator const_iterator;
		typedef typename std::vector<_Ty>::const_reverse_iterator const_reverse_iterator;

		priority_queue(const _Pr& compare) 
			: heap_vec(), comp(compare) {}
		explicit priority_queue(const _Pr& compare = _Pr()) 
			: heap_vec(), comp(compare) {}
		priority_queue(const priority_queue& _other)
			: heap_vec(_other.heap_vec), comp(_other.comp) {}
		priority_queue(priority_queue&& _other)
			: heap_vec(std::move(_other.heap_vec)), comp(std::move(_other.comp)) {}
		~priority_queue() {}
		priority_queue& operator=(const priority_queue& _other) {
			if (this != &_other)
				priority_queue(_other).swap(*this);
			return *this;
		}
		priority_queue& operator=(priority_queue&& _other) {
			if (this != &_other)
				priority_queue(std::move(_other)).swap(*this);
			return *this;
		}

		// Capacity

		bool empty() const noexcept {
			return heap_vec.empty();
		}
		size_type size() const noexcept {
			return heap_vec.size();
		}

		// Element Access

		const_reference top() const {
			return heap_vec[0];
		}
		std::ostream& write(std::ostream& _os, char _delim = ' ') {
			priority_queue<value_type, _Pr> tmp(*this);
			while (!tmp.empty()) {
				_os << tmp.top() << _delim;
				tmp.dequeue();
			}
			return _os;
		}

		// Modifiers

		void enqueue(const value_type& _val) {
			heap_vec.push_back(_val);
			bubble_up(heap_vec.size()-1);
		}
		template<class... Args>
		void emplace(Args&&... _args) {
			heap_vec.emplace_back(std::forward<Args>(args)...);
			bubble_up(heap_vec.size()-1);
		}
		void dequeue() {
			pop_top();
		}
		void clear() noexcept {
			heap_vec.clear();
		}
		const_reference find(const value_type& _val) const {
			return *std::find(heap_vec.begin(), heap_vec.end(), _val);
		}
		template<class UnaryPredicate>
		const_reference find(UnaryPredicate _p) const {
			return *std::find_if(heap_vec.begin(), heap_vec.end(), _p);
		}
		void alter(const std::pair<value_type, value_type>& _pair) {
			*std::find(heap_vec.begin(), heap_vec.end(), _pair.first) = _pair.second;
		}
		template<class UnaryPredicate>
		void alter(const value_type& _alter_to_val, UnaryPredicate _p) {
			*std::find_if(heap_vec.begin(), heap_vec.end(), _p) = _alter_to_val;
		}
		void swap(priority_queue& _other) {
			heap_vec.swap(_other.heap_vec);
			comp = _other.comp;
		}

		// Iterators

		const_iterator cbegin() const noexcept {
			return heap_vec.cbegin();
		}
		const_iterator cend() const noexcept {
			return heap_vec.cend();
		}
		const_reverse_iterator crbegin() const noexcept {
			return heap_vec.crbegin();
		}
		const_reverse_iterator crend() const noexcept {
			return heap_vec.crend();
		}

	private:
		std::vector<value_type> heap_vec;
		_Pr comp;

		void bubble_down(size_type _pos) {
			size_type heap_size = heap_vec.size();
			size_type left_child = 2 * _pos + 1;
			size_type right_child = 2 * _pos + 2;
			if (left_child >= heap_size) return;
			size_type min_pos = _pos;
			if (comp(heap_vec[_pos], heap_vec[_left_child]))
				min_pos = left_child;
			if (right_child < heap_size && comp(heap_vec[min_pos], heap_vec[right_child]))
				min_pos = right_child;
			if (min_pos != _pos) {
				std::swap(heap_vec[_pos], heap_vec[min_pos]);
				bubble_down(min_pos);
			}
		}
		void bubble_up(size_type _pos) {
			if (!_pos) return;
			size_type parent = (_pos - 1) / 2;
			if (comp(heap_vec[parent], heap_vec[_pos])) {
				std::swap(heap_vec[_pos], heap_vec[parent]);
				bubble_up(parent);
			}
		}
		void pop_top() {
			size_type heap_size = heap_vec.size();
			if (!heap_size) return;
			heap_vec[0] = heap_vec[--heap_size];
			heap_vec.pop_back();
			bubble_down(0);
		}
		void heapify() {
			for (size_type i = (heap_vec.size() - 1); i >= 0; --i)
				bubble_down(i);
		}
	};

	template<typename _Ty, class _Pr>
	std::ostream& operator<<(std::ostream& _os, const priority_queue<_Ty, _Pr>& _pq) {
		return _pq.write(_os);
	}

}

#endif // !PRIORITY_QUEUE_H
