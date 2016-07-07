#ifndef FILE_MANIPULATOR_H
#define FILE_MANIPULATOR_H
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace crsc {

	/**
	 * \class file_manipulator
	 *
	 * \brief A class to load file stream positions into memory during construction allowing
	 *	      quicker access for reading specified line numbers / blocks of lines as well as
	 *		  manipulating specific lines in the file.
	 *
	 * Only use this class if file_manipulator construction will be rare and if line numbers
	 * of the file to manipulate are unknown at compile time.
	 */
	class file_manipulator {
	public:
		/**
		 * \brief Initialises an instance of file_manipulator class with specified std::string filename.
		 *
		 * Loads the file with given name constructing a std::vector<std::fstream::streampos> containing
		 * the stream positions of each line beginning in the file.
		 *
		 * \param _filename Name/directory of file.
		 * \param _max_line_length Optional maximum length of file line, used for optimisation.
		 */
		file_manipulator(const std::string& _filename, std::size_t _max_line_length = static_cast<std::size_t>(256)) :
			fs(_filename), filename(_filename) {
			cache_file_streampos(_max_line_length);
		}
		/**
		 * \brief Initialises an instance of file_manipulator class with specified const char* filename.
		 *
		 * Loads the file with given name constructing a std::vector<std::fstream::streampos> containing
		 * the stream positions of each line beginning in the file.
		 *
		 * \param _filename Name/directory of file.
		 * \param _max_line_length Optional maximum length of file line, used for optimisation.
		 */
		file_manipulator(const char* _filename, std::size_t _max_line_length = static_cast<std::size_t>(256)) :
			fs(_filename), filename(_filename) {
			cache_file_streampos(_max_line_length);
		}
		/**
	 	 * \brief Deleted copy constructor, copy construction is forbidden.
		 */
		file_manipulator(const file_manipulator& _other) = delete;
		/**
		 * \brief Move constructor, uses move-semantics to move an instance of file_manipulator to this such
		 *        that the instance being moved is left in a valid but unspecified state.
		 *
		 * \param _other Instance of file_manipulator to move to this.
		 */
		file_manipulator(file_manipulator&& _other) :
			fs(std::move(_other.fs)), filename(std::move(_other.filename)), line_streampos_vec(std::move(_other.line_streampos_vec)) {
		}
		~file_manipulator() = default;
		/**
		 * \brief Reads a specified line of the file.
		 *
		 * \param _n Line number to read.
		 * \return std::string of specified line in file.
		 * \throws Throws std::out_of_range exception if _n exceeds lines in file.
		 */
		std::string read_line(std::size_t _n) {
			// check for out of bounds access
			if (_n >= line_streampos_vec.size() - 1)
				throw std::out_of_range("The file: " + filename + " does not have " + std::to_string(_n) + " lines.");
			navigate_to_line(_n);
			std::string rtn_str;
			// read this line into rtn_str and return it
			std::getline(fs, rtn_str);
			return rtn_str;
		}
		/**
		 * \brief Reads a specified block of lines of the file.
		 *
		 * \remark _start < _end, otherwise undefined behaviour.
		 * \param _start Starting line number to read from (inclusive).
		 * \param _end Ending line number to read to (inclusive).
		 * \return std::string of specified block of lines in file.
		 * \throws Throws std::out_of_range exception if _end exceeds lines in file.
		 * \throws Throws std::logic_error if _start > _end.
		 */
		std::string read_lines(std::size_t _start, std::size_t _end) {
			// check for logical inconsistency
			if (_start > _end)
				throw std::logic_error("_start must be <= _end");
			// check for out of bounds access
			if (_end >= line_streampos_vec.size() - 1)
				throw std::out_of_range("The file: " + filename + " does not have " + std::to_string(_end) + " lines.");
			if (_start == _end)
				return read_line(_start);
			std::string rtn_str = "";
			navigate_to_line(_start);
			// loop over specified file block getting the line contents
			// and appending to the return string
			for (std::size_t i = _start; i <= _end; ++i) {
				std::string temp;
				std::getline(fs, temp);
				rtn_str += temp;
			}
			return rtn_str;
		}
		/**
		 * \brief Overwrites a specified line in the file with a given std::string.
		 *
		 * \param _n Line number to overwrite.
		 * \param _str Instance of std::string to overwrite line with.
		 * \throws Throws std::out_of_range exception if _n exceeds lines in file.
		 */
		void overwrite_line(std::size_t _n, const std::string& _str) {
			// check for out of bounds access
			if (_n >= line_streampos_vec.size() - 1)
				throw std::out_of_range("The file: " + filename + " does not have " + std::to_string(_n) + " lines.");
			// go to given line in file
			navigate_to_line(_n);
			// read line contents
			std::string line_str;
			std::getline(fs, line_str);
			// std::string to write
			std::string wrt_str = _str;
			// if current line contents size is larger than std::string to write
			// then append line_str.size() - _str.size() spaces to wrt_str
			if (_str.size() < line_str.size()) {
				wrt_str += std::string(" ", line_str.size() - _str.size());
			}
			// re-navigate to given line
			navigate_to_line(_n);
			fs << wrt_str;
		}
		/**
		 * \brief Deletes a specified line in the file such that the line is empty.
		 *
		 * \remark This method does not delete and shift the line, it only erases the contents
		 *         of the given line (replacing the contents with whitespace).
		 * \param _n Line number to delete.
		 */
		void erase_line_contents(std::size_t _n) {
			overwrite_line(_n, " ");
		}
		/**
		 * \brief Adds a new blank line in the file before a given line number.
		 *
		 * \param _n Line to add blank line before.
		 * \throws Throws std::out_of_range exception if _n exceeds lines in file.
		 */
		void add_line(std::size_t _n) {
			if (_n >= line_streampos_vec.size() - 1)
				throw std::out_of_range("The file: " + filename + " does not have " + std::to_string(_n) + " lines.");
			navigate_to_line(_n);
			fs << "\n";
		}

		/**
		 * \brief Deleted copy assignment operator, copy assignment is forbidden.
		 */
		file_manipulator& operator=(const file_manipulator& _other) = delete;
		/**
		 * \brief Move assignment operator, uses move-semantics to move the parameterised
		 *		  file_manipulator instance to this. Instance being moved is left in a
		 *        valid but unspecified state.
		 *
		 * \param _other rvalue reference to file_manipulator instance.
		 */
		file_manipulator& operator=(file_manipulator&& _other) {
			// check for self-assignment
			if (this != &_other) {
				fs = std::move(_other.fs);
				line_streampos_vec = std::move(_other.line_streampos_vec);
				filename = std::move(_other.filename);
			}
			return *this;
		}
	private:
		std::fstream fs;
		std::vector<std::fstream::streampos> line_streampos_vec;
		std::string filename;
		/**
		 * \brief Loads the file streampos elements into line_streampos_vec field.
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