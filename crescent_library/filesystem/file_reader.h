#ifndef FILE_MANIPULATOR_H
#define FILE_MANIPULATOR_H
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace crsc {
	/**
	 * \class file_reader
	 *
	 * \brief A class to load file stream positions into memory during construction allowing
	 *	      quicker access for reading specified line numbers / blocks of lines as well as
	 *		  manipulating specific lines in the file.
	 *
	 * \author Samuel Rowlinson
	 * \date June, 2016
	 */
	class file_reader {
	public:
		// CONSTRUCTION/ASSIGNMENT
		/**
		 * \brief Initialises an instance of the `file_reader` class with specified `std::string` filename.
		 *
		 * Loads the file with given name constructing a std::vector<std::fstream::streampos> containing
		 * the stream positions of each line beginning in the file.
		 *
		 * \param _filename Name/directory of file.
		 * \param _max_line_length Optional maximum length of file line, used for optimisation.
		 */
		explicit file_reader(const std::string& _filename, std::size_t _max_line_length = 256U) 
			: fs(_filename), filename(_filename) { cache_file_streampos(_max_line_length); }
		/**
	 	 * \brief Deleted copy constructor, copy constring is forbidden. No two `file_reader` instances
		 *        may observe the same loaded file stream resource.
		 */
		file_reader(const file_reader&) = delete;
		/**
		 * \brief Move constructor, uses move-semantics to move an instance of `file_reader` to this such
		 *        that the instance being moved is left in a valid but unspecified state.
		 *
		 * \param _other Instance of `file_reader` to move to this.
		 */
		file_reader(file_reader&& _other) :
			fs(std::move(_other.fs)), filename(std::move(_other.filename)), 
				line_streampos_vec(std::move(_other.line_streampos_vec)) {
		}
		/**
		 * \brief Deleted copy assignment operator, copy assignment is forbidden. No two `file_reader` 
		 *        instances may observe the same loaded file stream resource.
		 */
		file_reader& operator=(const file_reader& _other) = delete;
		/**
		 * \brief Move assignment operator, uses move-semantics to move the parameterised
		 *		  `file_reader` instance to this. Instance being moved is left in a
		 *        valid but unspecified state.
		 *
		 * \param _other rvalue reference to `file_reader` instance.
		 */
		file_reader& operator=(file_reader&& _other) {
			// check for self-assignment
			if (this != &_other) {
				fs = std::move(_other.fs);
				line_streampos_vec = std::move(_other.line_streampos_vec);
				filename = std::move(_other.filename);
			}
			return *this;
		}
		// CAPACITY
		/**
		 * \brief Returns the number of lines in the file.
		 *
		 * \return Number of lines in the file.
		 */
		std::size_t lines() const noexcept {
			return line_streampos_vec.size();
		}
		/**
		 * \brief Checks if the file is empty.
		 *
		 * \return `true` if file is empty, `false` otherwise.
		 */
		bool empty() const noexcept {
			return line_streampos_vec.empty();
		}
		// CONTENT ACCESS
		/**
		 * \brief Reads a specified line of the file.
		 *
		 * \param n Line number to read.
		 * \return `std::string` of specified line in file.
		 * \throws Throws `std::out_of_range` exception if `!(n < lines())`.
		 */
		std::string read_line(std::size_t n) {
			// check for out of bounds access
			if (n >= line_streampos_vec.size() - 1)
				throw std::out_of_range("The file: " + filename + " does not have " + std::to_string(n) + " lines.");
			navigate_to_line(n); // navigate to line n in fs
			std::string rtn_str;
			std::getline(fs, rtn_str);
			return rtn_str;
		}
		/**
		 * \brief Reads the first line of the file.
		 *
		 * \return `std::string` of first line in file.
		 */
		std::string first_line() {
			navigate_to_line(0); // navigate to line 0 in fs
			std::string fst_line_str;
			std::getline(fs, fst_line_str);
			return fst_line_str;
		}
		/**
		 * \brief Reads the last line of the file.
		 *
		 * \return `std::string` of last line in file.
		 */
		std::string last_line() {
			navigate_to_line(line_streampos_vec.size() - 1);  // navigate to last line in fs
			std::string lst_line_str;
			std::getline(fs, lst_line_str);
			return lst_line_str;
		}
	private:
		std::fstream fs;
		std::vector<std::fstream::streampos> line_streampos_vec;	// internal stream position container
		std::string filename;
		/**
		 * \brief Loads the file `std::streampos` elements into the internal stream position container.
		 *
		 * \param _max_line_length Maximum length of file line, used for optimisation.
		 */
		void cache_file_streampos(std::size_t _max_line_length) {
			std::string s;
			// reserve space for performance
			s.reserve(_max_line_length);
			// loop over input filestream
			while (fs) {
				// push back std::streampos elements of file to line_streampos_vec
				line_streampos_vec.push_back(fs.tellg());
				std::getline(fs, s);
			}
		}
		/**
		 * \brief Navigates to the given line of the file stream.
		 *
		 * \param _n Line number to navigate to.
		 */
		void navigate_to_line(std::size_t _n) {
			// clear all error state and EOF flags
			fs.clear();
			// go to specified line in file using access of line_streampos_vec
			fs.seekg(line_streampos_vec[_n]);
		}
	};

}

#endif 