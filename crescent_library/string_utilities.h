#ifndef STRING_UTILITIES_H
#define STRING_UTILITIES_H
#include <algorithm>
#include <cctype>
#include <set>
#include <string>
#include <sstream>
#include <vector>

namespace crsc {
	/**
	 * \brief Splits a `std::string` around a given delimiter into a `std::vector<std::string>`.
	 *
	 * \param s Instance of `std::string` to split.
	 * \param delim `char` to split `s` around.
	 * \return A `std::vector` of `std::string` instances containing each sub-string after splits.
	 */
	std::vector<std::string> split(const std::string& s, char delim) {
		std::vector<std::string> elems;
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim))
			elems.push_back(item);
		return elems;
	}
	/**
	 * \brief Splits and parses a `std::string` around delimiters `delim` into a `std::vector<int>`.
	 * \param s `std::string` to split and parse.
	 * \param delim delimiter around which to split `s`.
	 * \return `std::vector<int>` of split and parsed values.
	 */
	std::vector<int> split_stoi(const std::string& s, char delim) {
		std::vector<int> elems;
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim))
			elems.push_back(std::stoi(item));
		return elems;
	}
    /**
	 * \brief Splits and parses a `std::string` around delimiters `delim` into a `std::vector<long>`.
	 * \param s `std::string` to split and parse.
	 * \param delim delimiter around which to split `s`.
	 * \return `std::vector<long>` of split and parsed values.
	 */
	std::vector<long> split_stol(const std::string& s, char delim) {
		std::vector<long> elems;
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim))
			elems.push_back(std::stol(item));
		return elems;
	}
    /**
	 * \brief Splits and parses a `std::string` around delimiters `delim` into a `std::vector<long long>`.
	 * \param s `std::string` to split and parse.
	 * \param delim delimiter around which to split `s`.
	 * \return `std::vector<long long>` of split and parsed values.
	 */
	std::vector<long long> split_stoll(const std::string& s, char delim) {
		std::vector<long long> elems;
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim))
			elems.push_back(std::stoll(item));
		return elems;
	}
    /**
	 * \brief Splits and parses a `std::string` around delimiters `delim` into a `std::vector<unsigned long>`.
	 * \param s `std::string` to split and parse.
	 * \param delim delimiter around which to split `s`.
	 * \return `std::vector<unsigned long>` of split and parsed values.
	 */
	std::vector<unsigned long> split_stoul(const std::string& s, char delim) {
		std::vector<unsigned long> elems;
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim))
			elems.push_back(std::stoul(item));
		return elems;
	}
    /**
	 * \brief Splits and parses a `std::string` around delimiters `delim` into a `std::vector<unsigned long long>`.
	 * \param s `std::string` to split and parse.
	 * \param delim delimiter around which to split `s`.
	 * \return `std::vector<unsigned long long>` of split and parsed values.
	 */
	std::vector<unsigned long long> split_stoull(const std::string& s, char delim) {
		std::vector<unsigned long long> elems;
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim))
			elems.push_back(std::stoull(item));
		return elems;
	}
    /**
	 * \brief Splits and parses a `std::string` around delimiters `delim` into a `std::vector<float>`.
	 * \param s `std::string` to split and parse.
	 * \param delim delimiter around which to split `s`.
	 * \return `std::vector<float>` of split and parsed values.
	 */
	std::vector<float> split_stof(const std::string& s, char delim) {
		std::vector<float> elems;
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim))
			elems.push_back(std::stof(item));
		return elems;
	}
    /**
	 * \brief Splits and parses a `std::string` around delimiters `delim` into a `std::vector<double>`.
	 * \param s `std::string` to split and parse.
	 * \param delim delimiter around which to split `s`.
	 * \return `std::vector<double>` of split and parsed values.
	 */
	std::vector<double> split_stod(const std::string& s, char delim) {
		std::vector<double> elems;
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim))
			elems.push_back(std::stod(item));
		return elems;
	}
    /**
	 * \brief Splits and parses a `std::string` around delimiters `delim` into a `std::vector<long double>`.
	 * \param s `std::string` to split and parse.
	 * \param delim delimiter around which to split `s`.
	 * \return `std::vector<long double>` of split and parsed values.
	 */
	std::vector<long double> split_stold(const std::string& s, char delim) {
		std::vector<long double> elems;
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim))
			elems.push_back(std::stold(item));
		return elems;
	}
	/**
	 * \brief Prepends a `std::string` with a given `std::string`.
	 *
	 * \param s Instance of `std::string` to modify.
	 * \param pr `std::string` to prepend `s` with.
	 * \return Reference to `s` with prepended string `pr`.
	 */
	std::string& prepend(std::string& s, const std::string& pr) {
		return s.insert(0, pr);
	}
	/**
	 * \brief Determines whether a `std::string` instance starts with a given `char`.
	 *
	 * \param _s Instance of `std::string`.
	 * \param _c `char` to check against.
	 * \return `true` if `_s` starts with `_c`, `false` otherwise.
	 */
	bool starts_with(const std::string& s, char c) {
		return *s.cbegin() == c;
	}
	/**
	 * \brief Determines whether a `std::string` instance ends with a given `char`.
	 *
	 * \param _s Instance of `std::string`.
	 * \param _c `char` to check against.
	 * \return `true` if `_s` ends with `_c`, `false` otherwise.
 	 */
	bool ends_with(const std::string& s, char c) {
		return *--(s.cend()) == c;
	}
	/**
	 * \brief Converts all `char`s in a `std::string` to upper case.
	 *
	 * \param _s Reference to `std::string` instance.
	 * \return Reference to `_s` with all `char`s converted to upper case.
	 */
	std::string& to_upper(std::string& s) {
		for (auto& c : s)
			c = static_cast<char>(std::toupper(c));
		return s;
	}
	/**
	 * \brief Converts all `char`s in a `std::string` to lower case.
	 *
	 * \param _s Reference to `std::string` instance.
	 * \return Reference to `_s` with all `char`s converted to lower case.
	 */
	std::string& to_lower(std::string& s) {
		for (auto& c : s)
			c = static_cast<char>(std::tolower(c));
		return s;
	}
	/**
	 * \brief Removes all whitespaces from a `std::string`.
	 *
	 * \param _s Reference to `std::string` instance.
	 * \return Reference to `_s` with all whitespace `char`s removed.
	 */
	std::string& remove_whitespace(std::string& s) {
		// invoke erase-remove idiom and use std::isspace(char) to check for whitespaces.
		s.erase(std::remove_if(s.begin(), s.end(), [](char x) { return std::isspace(x); }), s.end());
		return s;
	}
	/**
	 * \brief Removes all instances of any `char` in a given `std::set<char>` from a `std::string`.
	 *
	 * \param _s Reference to `std::string` instance.
	 * \param _char_set A `std::set` containing instances of `char` to remove from `_s`.
	 * \return Reference to `_s` with all instances of any `char` within `_char_set` removed.
	 */
	std::string& trim(std::string& s, const std::set<char>& char_set) {
		// invoke erase-remove idiom and pass _char_set to lambda captue for checking if char's occur
		s.erase(std::remove_if(s.begin(), s.end(), [&char_set](char x) {return char_set.find(x) != char_set.end(); }), s.end());
		return s;
	}
	/**
	 * \brief Removes all instances of any `char` in a given `std::string` from a `std::string`.
	 */
	std::string& trim(std::string& s, const std::string& str_source) {
		s.erase(std::remove_if(s.begin(), s.end(), [&str_source](char x) {return str_source.find(x) != std::string::npos; }), s.end());
		return s;
	}
	/**
	 * \brief Removes all vowel characters from a `std::string`.
	 *
	 * \param _s Reference to `std::string` instance.
	 * \param Reference to `_s` with all vowels removed.
	 */
	std::string& remove_vowels(std::string& s) {
		std::set<char> vowel_set = { 'a', 'e', 'i', 'o', 'u', 'A', 'E', 'I', 'O', 'U' };
		return trim(s, vowel_set);
	}
	/**
	 * \brief Removes all leading whitespace `char`s from a `std::string` instance.
	 *
	 * \param _s Reference to `std::string` instance.
	 * \return Reference to `_s` with all leading whitespaces removed.
	 */
	std::string& remove_leading_whitespaces(std::string& s) {
		auto it = s.begin();
		// loop until past-the-end iterator of _s
		while (it != s.end()) {
			// if character is a space, erase it and set it to
			// next valid std::string::iterator in _s
			if (std::isspace(*it)) {
				it = s.erase(it);
			}
			// if character is not a space, finished so break
			else
				break;
		}
		return s;
	}
	/**
	 * \brief Removes all trailing whitespace `char`s from a `std::string` instance.
	 *
	 * \param _s Reference to `std::string` instance.
	 * \return Reference to `_s` with all trailing whitespaces removed.
 	 */
	std::string& remove_trailing_whitespaces(std::string& s) {
		auto it = s.rbegin();
		// loop until rend iterator of _s
		while (it != s.rend()) {
			// if character is a space, increment the iterator,
			// erase the space and set it to next valid reverse
			// iterator of std::string _s 
			if (std::isspace(*it)) {
				++it;
				it = std::reverse_iterator<std::string::iterator>(s.erase(it.base()));
			}
			// if character is not a space, finished so break
			else
				break;
		}
		return s;
	}
}

#endif