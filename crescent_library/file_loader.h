#ifndef FILE_LOADER_H
#define FILE_LOADER_H
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace crsc {
	/**
	 * \class file_loader
	 *
	 * \brief A class to load file contents into memory allowing easier accessing and manipulation of
	 *        file data. Only use for relatively small files as the entire file contents are loaded into
	 *		  memory during construction of a file_loader object.
	 *
	 * \tparam Container The type of the underlying container used for the internal cached storage. This
	 *         container must satisfy the requirements of `SequenceContainer` and its iterators must 
	 *         satisfy the requirements of `RandomAccessIterator` (see C++ Concepts). Additionally the
	 *         `Container::value_type` must be `std::string`. If these condtions are not satisfied then
	 *         undefined behaviour is invoked for all methods and operations involving `file_loader`.
	 * \remark In the documentation of this class, the term "internal cached storage" is used to
	 *         denote the encapsulated data structure which stores the contents of the file in
	 *         memory. This structure is used for reading and manipulating the contents data
	 *         internally - any changes made to this structure are not automatically applied to
	 *         the filestream (i.e. the file itself), these updates can be pushed by invoking
	 *		   file_loader::write_changes() on an instance of a file_loader.
	 * \author Samuel Rowlinson
	 * \date June, 2016
	 */
	template<
		class Container = std::vector<std::string>
	> class file_loader {
		typedef Container::const_iterator const_iterator;
		typedef Container::iterator iterator;
		typedef Container::const_reverse_iterator const_reverse_iterator;
		typedef Container::reverse_iterator reverse_iterator;
	public:
		// CONSTRUCTION/ASSIGNMENT
		/**
		 * \brief Constructs a file_loader instance using a given filename, loads the file
		 *        with name `_filename` and caches its contents into the internal cached
		 *        storage.
		 *
		 * \param _filename Name/directory of file to load.
		 * \param _max_line_length Optional, approximate maximum length of file lines.
		 */
		explicit file_loader(const std::string& _filename, std::size_t _max_line_length = 256U) 
			: fs(_filename), filename(_filename) { cache_contents(_max_line_length); }
		/**
		 * \brief Deleted copy construction, copy constructing is forbidden. No two `file_loader`
		 *        instances may manage the same loaded file stream resource.
		 */
		file_loader(const file_loader&) = delete;
		/**
		 * \brief Move constructor, moves a given file_loader instance to this leaving the
		 *        parameterised instance in a valid but unspecified state.
		 *
		 * \param _other rvalue reference to a file_loader instance to move.
		 */
		file_loader(file_loader&& _other) 
			: fs(std::move(_other.fs)), filename(std::move(_other.filename)), 
				cached_contents_cntr(std::move(_other.cached_contents_cntr)) {}
		/**
		 * \brief Deleted copy assignment operator, copy assignment is forbidden. No two `file_loader`
		 *        instances may manage the same loaded file stream resource.
		 */
		file_loader& operator=(const file_loader&) = delete;
		/**
		 * \brief Move assignment operator, uses move-semantics to move the parameterised
		 *		  file_loader instance to this. Instance being moved is left in a
		 *        valid but unspecified state.
		 *
		 * \param _other rvalue reference to file_loader instance.
		 */
		file_loader& operator=(file_loader&& _other) {
			// check for self-assignment
			if (this != &_other) {
				fs = std::move(_other.fs);
				filename = std::move(_other.filename);
				cached_contents_cntr = std::move(_other.cached_contents_cntr);
			}
			return *this;
		}
		// CAPACITY
		/**
		 * \brief Returns the number of lines in the internal container.
		 *
		 * \return Number of lines in current state of internal cached storage.
		 */
		std::size_t lines() const noexcept {
			return cached_contents_cntr.size();
		}
		/**
		 * \brief Checks if the internal container is empty.
		 *
		 * \return `true` if internal cached storage container is empty, `false` otherwise.
		 */
		bool empty() const noexcept {
			return cached_contents_cntr.empty();
		}
		// CONTENT ACCESS
		/**
		 * \briefs Gets a const reference to the `std::string` contents of line `n` of
		 *         the internal cached storage. Provides read-only access to this line
		 *         of the always up to date internal container.
		 *
		 * \param n Line number to read.
		 * \return const reference to `std::string` instance given by `n`'th line.
		 * \throw Throws `std::out_of_range` exception if `!(n < lines())`.
		 */
		const std::string& line_at(std::size_t n) const {
			if (!(n < cached_contents_cntr.size()))
				throw std::out_of_range("File: " + filename + " does not have " + std::to_string(n) + " lines.");
			return cached_contents_cntr[n];
		}
		/**
		 * \briefs Gets a reference to the `std::string` contents of line `n` of
		 *         the internal cached storage. Provides read and write access
		 *         to this line of the always up to date internal container.
		 *
		 * \remark Any changes made to this line externally will be applied to the file
		 *         associated with this class only when calling `write_changes`.
		 * \param n Line number to read.
		 * \return reference to `std::string` instance given by `n`'th line.
		 * \throw Throws `std::out_of_range` exception if `!(n < lines())`.
		 */
		std::string& line_at(std::size_t n) {
			if (!(n < cached_contents_cntr.size()))
				throw std::out_of_range("File: " + filename + " does not have " + std::to_string(n) + " lines.");
			return cached_contents_cntr[n];
		}
		/**
		 * \briefs Gets a const reference to the `std::string` contents of line `n` of
		 *         the internal cached storage. Provides read-only access to this line
		 *         of the always up to date internal container.
		 * 
		 * This method does not perform bounds-checking, attempting to access a line number
		 * `m` for which `!(m < lines())` results in undefined behaviour.
		 * 
		 * \param n Line number to read.
		 * \return const reference to `std::string` instance given by `n`'th line.
		 */
		const std::string& operator[](std::size_t n) const {
			return cached_contents_cntr[n];
		}
		/**
		 * \briefs Gets a reference to the `std::string` contents of line `n` of
		 *         the internal cached storage. Provides read and write access
		 *         to this line of the always up to date internal container.
		 *
		 * This method does not perform bounds-checking, attempting to access a line number
		 * `m` for which `!(m < lines())` results in undefined behaviour.
		 *
		 * \remark Any changes made to this line externally will be applied to the file
		 *         associated with this class only when calling `write_changes`.
		 * \param n Line number to read.
		 * \return reference to `std::string` instance given by `n`'th line.
		 */
		std::string& operator[](std::size_t n) {
			return cached_contents_cntr[n];
		}
		/**
		 * \brief Returns const reference to the first line of the internal cached storage.
		 *
		 * \return First line of the internal container.
		 */
		const std::string& front() const noexcept {
			return cached_contents_cntr.front();
		}
		/**
	 	 * \brief Returns reference to the first line of the internal cached storage.
		 *
		 * \return First line of the internal container.
		 */
		std::string& front() noexcept {
			return cached_contents_cntr.front();
		}
		/**
		 * \brief Returns const reference to the last line of the internal cached storage.
		 *
		 * \return Last line of the internal container.
		 */
		const std::string& back() const noexcept {
			return cached_contents_cntr.back();
		}
		/**
		 * \brief Returns reference to the last line of the internal cached storage.
		 *
		 * \return Last line of the internal container.
		 */
		std::string& back() noexcept {
			return cached_contents_cntr.back();
		}
		// INTERNAL CACHE MODIFIERS
		/**
		 * \brief Clears the entire contents of the internal cached storage such that the
		 *        next call to `write_changes` wipes the file completely.
		 */
		void clear_contents() noexcept {
			cached_contents_cntr.clear();
		}
		/**
		 * \brief Erases a given line of the interal cached storage via iterator position
		 *        such that the next call to `write_changes` will remove the line from the
		 *        file contents.
		 *
		 * \param pos Position of line to erase in internal container.
		 * \return Iterator to next valid position in internal cached storage container.
		 */
		iterator erase_line(const_iterator pos) {
			return cached_contents_cntr.erase(pos);
		}
		/**
		 * \brief Erases a block of lines from the internal cached storage via iterator
		 *        positions such that the next call to `write_changes` will remove the
		 *        block of lines from the file contents.
		 *
		 * \param first Position of first line of block to erase.
		 * \param last Position of last line of block to erase.
		 * \return Iterator to next valid position in internal cached storage container.
		 */
		iterator erase_block(const_iterator first, const_iterator last) {
			return cached_contents_cntr.erase(first, last);
		}
		/**
		 * \brief Inserts a line with the given contents before iterator position `pos` in
		 *        the internal cached storage such that the next call to `write_changes` will
		 *        add this line to the file contents.
		 *
		 * \param pos Position of line after insertion position.
		 * \param str Contents of added line.
		 */
		iterator insert_line(const_iterator pos, const std::string& str) {
			return cached_contents_cntr.insert(pos, str);
		}
		/**
		 * \brief Inserts a line with the given contents before iterator position `pos` in
		 *        the internal cached storage via move-semantics such that the next call
		 *        to `write_changes` will add this line to the file contents.
		 *
		 * \param pos Position of line after insertion position.
		 * \param str Contents of added line.
		 */
		iterator insert_line(const_iterator pos, std::string&& str = "") {
			return cached_contents_cntr.insert(pos, std::move(str));
		}
		/**
		 * \brief Inserts a block of lines to the internal cached storage such that the next
		 *        call to `write_changes` will add this block to the file contents.
		 *
		 * \param pos Position of line after first insertion position.
		 * \param first Iterator to first position of block to insert.
		 * \param last Iterator to last position of block to insert.
		 */
		template<class InputIt>
		iterator insert_block(const_iterator pos, InputIt first, InputIt last) {
			return cached_contents_cntr.insert(pos, first, last);
		}
		/**
		 * \brief Pushes a line with the given contents to the back of the internal cached
		 *        storage such that the next call to `write_changes` will add this line to
		 *        the end of the file contents.
		 *
		 * \param str Contents of added line.
		 */
		void push_line_back(const std::string& str) {
			cached_contents_cntr.push_back(str);
		}
		/**
		 * \brief Pushes a line with the given contents to the back of the internal cached
		 *        storage via move-semantics such that the next call to `write_changes` will 
		 *        add this line to the end of the file contents.
		 *
		 * \param str Contents of added line.
		 */
		void push_line_back(std::string&& str = "") {
			cached_contents_cntr.push_back(std::move(str));
		}
		/**
		 * \brief Pops the last line from the internal cached storage such that the next call
		 *        to `write_changes` removes the last line from the file contents.
		 */
		void pop_line_back() {
			cached_contents_cntr.pop_back();
		}
		// ITERATORS
		const_iterator cbegin() const noexcept {
			return cached_contents_cntr.cbegin();
		}
		iterator begin() noexcept {
			return cached_contents_cntr.begin();
		}
		const_iterator cend() const noexcept {
			return cached_contents_cntr.cend();
		}
		iterator end() noexcept {
			return cached_contents_cntr.end();
		}
		const_reverse_iterator crbegin() const noexcept {
			return cached_contents_cntr.crbegin();
		}
		reverse_iterator rbegin() noexcept {
			return cached_contents_cntr.rbegin();
		}
		const_reverse_iterator crend() const noexcept {
			return cached_contents_cntr.crend();
		}
		reverse_iterator rend() noexcept {
			return cached_contents_cntr.rend();
		}
		// FILE OPERATIONS/MODIFIERS
		/**
		 * \brief Writes all changes made to the internal cached storage to the
		 *		  filestream, overwriting the current contents of the file.
		 */
		void write_changes() {
			fs.clear();
			for (const auto& el : cached_contents_cntr) {
				fs << el << '\n';
			}
		}
		/**
		 * \brief Gets a const reference to the internal cached storage container
		 *        holding the line-by-line contents of the current state of the
		 *        `file_loader` instance.
		 *
		 * \return const reference to the internal cached storage container.
		 */
		const Container& file_contents() const noexcept {
			return cached_contents_cntr;
		}
	private:
		std::fstream fs;
		std::string filename;
		Container cached_contents_cntr;	// internal cached storage container
		/**
		 * \brief Caches contents of files into the internal cached storage container.
		 *
		 * \param _max_line_length Approximate maximum line length of the file, used for performance improvements.
		 */
		void cache_contents(std::size_t _max_line_length) {
			std::string line_str;
			// reserve space for performance
			line_str.reserve(_max_line_length);
			while (std::getline(fs, line_str)) {
				cached_contents_cntr.push_back(line_str);
			}
		}
	};
}

#endif // !FILE_LOADER_H