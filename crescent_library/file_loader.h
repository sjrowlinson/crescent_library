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
	* \brief A class to load file contents into memory allowing easier accessing and manipulation
	*        of file data. Only use for small files as the entire file contents are loaded into
	*		  memory during construction of a file_loader object.
	*
	* \remark In the documentation of this class, the term "internal cached storage" is used to
	*         denote the encapsulated data structure which stores the contents of the file in
	*         memory. This structure is used for reading and manipulating the contents data
	*         internally - any changes made to this structure are not automatically applied to
	*         the filestream (i.e. the file itself), these updates can be pushed by invoking
	*		   file_loader::write_changes() on an instance of a file_loader.
	*/
	class file_loader {
	public:
		/**
		* \brief Constructs a file_loader instance using a given filename. Caches the file contents
		*		  into the internal cached storage container.
		*
		* \param _filename Name/directory of file to load.
		* \param _max_line_length Optional, approximate maximum length of file lines.
		*/
		file_loader(const std::string& _filename, std::size_t _max_line_length = static_cast<std::size_t>(256)) : fs(_filename), filename(_filename) {
			cache_contents(_max_line_length);
		}
		/**
		* \brief Deleted copy construction, copy constructing is forbidden.
		*/
		file_loader(const file_loader& _other) = delete;
		/**
		* \brief Move constructor, moves a given file_loader instance to this leaving the
		*        parameterised instance in a valid but unspecified state.
		*
		* \param _other rvalue reference to a file_loader instance to move.
		*/
		file_loader(file_loader&& _other) : fs(std::move(_other.fs)), filename(std::move(_other.filename)),
			cached_contents_vec(std::move(_other.cached_contents_vec)) {
		}
		~file_loader() = default;

		/**
		* \brief Gets the number of lines in the file.
		*
		* \return Number of lines in current state of internal cached storage.
		*/
		std::size_t lines() const noexcept {
			return cached_contents_vec.size();
		}
		/**
		* \brief Checks if the file is empty.
		*
		* \return True if internal cached storage container is empty, false otherwise.
		*/
		bool empty() const noexcept {
			return cached_contents_vec.empty();
		}
		/**
		* \brief Writes all changes made to the internal cached storage to the
		*		  filestream, overwriting the current contents of the file.
		*/
		void write_changes() {
			fs.clear();
			write_cache();
		}
		/**
		* \brief Reads a given line of the internal cached storage. This internal
		*		  store is guaranteed to always be up to date, no call to write_changes
		*        is required to maintain consistency.
		*
		* \param _n Line number to read.
		* \return const reference to std::string instance given by _n'th line.
		* \throws Throws std::out_of_range exception if _n exceeds lines in internal cached storage.
		*/
		const std::string& read_line(std::size_t _n) const {
			if (_n >= cached_contents_vec.size())
				throw std::out_of_range("File: " + filename + " does not have " + std::to_string(_n) + " lines.");
			return cached_contents_vec[_n];
		}
		/**
		* \brief Reads a block of lines of the internal cached storage where the
		*        start and end of the block are parameterised. The internal store
		*        is guaranteed to always be up to date, no call to write_changes
		*		  is required to maintain consistency.
		*
		* \param _n Beginning line number to read from (inclusive).
		* \param _m Ending line number to read to (inclusive).
		* \return A std::string instance containing block of lines data.
		* \throw Throws std::logic_error if _n > _m.
		* \throw Throws std::out_of_range exception if _n exceeds lines in internal cached storage.
		*/
		std::string read_lines(std::size_t _n, std::size_t _m) const {
			// check for logical inconsistency
			if (_n > _m)
				throw std::logic_error("_n must be <= _m");
			// check for out of bound access
			if (_m >= cached_contents_vec.size())
				throw std::out_of_range("File: " + filename + " does not have " + std::to_string(_m) + " lines.");
			if (_n == _m)
				return read_line(_n);
			std::string rtn_str = "";
			// loop over the given block and append contents to rtn_str
			for (std::size_t i = _n; i <= _m; ++i)
				rtn_str += cached_contents_vec[i] + "\n";
			return rtn_str;
		}
		/**
		* \brief Overwrites a given line of the internal cached storage such that
		*		  the next call to write_changes will update the file contents.
		*
		* \param _n Line number to overwrite.
		* \param _str std::string instance to overwrite current line contents with.
		* \throws Throws std::out_of_range exception if _n exceeds lines in internal cached storage.
		*/
		void overwrite_line(std::size_t _n, const std::string& _str) {
			if (_n >= cached_contents_vec.size())
				throw std::out_of_range("File: " + filename + " does not have " + std::to_string(_n) + " lines.");
			cached_contents_vec[_n] = _str;
		}
		/**
		* \brief Erases a given line of the internal cached storage such that
		*        the next call to write_changes will update the file contents.
		*
		* \param _n Line number to erase.
		* \return Iterator to next valid position in internal cached storage container.
		*/
		auto& erase_line(std::size_t _n) {
			return cached_contents_vec.erase(cached_contents_vec.begin() + _n);
		}
		/**
		* \brief Adds a line with given contents before position _n in the internal cached storage.
		*
		* \param _n Line number to add line before.
		* \param _str Contents of added line.
		*/
		auto& add_line(std::size_t _n, const std::string& _str = "") {
			return cached_contents_vec.insert(cached_contents_vec.begin() + _n, _str);
		}
		/**
		* \brief Adds a line with given contents before position _n in the internal cached storage.
		*
		* \param _n Line number to add line before.
		* \param _str rvalue reference to contents of added line.
		*/
		auto& add_line(std::size_t _n, std::string&& _str) {
			return cached_contents_vec.insert(cached_contents_vec.begin() + _n, std::forward<std::string>(_str));
		}

		auto& add_line(std::size_t _n, std::size_t _count, const std::string& _str = "") {
			return cached_contents_vec.insert(cached_contents_vec.begin() + _n, _count, _str);
		}
		template<class InputIt,
			class = std::enable_if_t<std::_Is_iterator<InputIt>::value>
		> auto& insert(std::size_t _n, InputIt _first, InputIt _last) {
			return cached_contents_vec.insert(cached_contents_vec.begin() + _n, _first, _last);
		}

		/**
		* \brief Deleted copy assignment operator, copy assignment is forbidden.
		*/
		file_loader& operator=(const file_loader& _other) = delete;
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
				cached_contents_vec = std::move(_other.cached_contents_vec);
			}
			return *this;
		}
	private:
		std::fstream fs;
		std::string filename;
		std::vector<std::string> cached_contents_vec;	// internal cached storage container

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
				cached_contents_vec.push_back(line_str);
			}
		}

		/**
		* \brief Convenience method for writing a std::vector<std::string> to an std::ostream instance.
		*/
		void write_cache() {
			for (const auto& x : cached_contents_vec) {
				fs << x << "\n";
			}
		}

	};

}

#endif