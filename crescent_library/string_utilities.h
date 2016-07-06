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
	* \brief Splits a std::string around a given delimiter into a given std::vector of std::string's.
	*
	* \param _s Instance of std::string to split.
	* \param _delim Character to split _s around.
	* \param _elems std::vector of std::string's to store split substrings.
	* \return Reference to _elems containing split substrings.
	*/
	std::vector<std::string>& split(const std::string& _s, char _delim, std::vector<std::string>& _elems) {
		std::stringstream ss(_s);
		std::string item;
		// store each line from ss split around _delim char into item
		while (std::getline(ss, item, _delim))
			_elems.push_back(item);
		return _elems;
	}

	/**
	* \brief Splits a std::string around a given delimiter into a std::vector of std::string's.
	*
	* \param _s Instance of std::string to split.
	* \param _delim Character to split _s around.
	* \return A std::vector of std::string's containing each sub-string after splits.
	*/
	std::vector<std::string> split(const std::string& _s, char _delim) {
		std::vector<std::string> elems;
		return split(_s, _delim, elems);
	}

	/**
	* \brief Prepends a std::string with a given std::string.
	*
	* \param _s Instance of std::string to modify.
	* \param _pr std::string to prepend _s with.
	* \return Reference to _s with prepended string _pr.
	*/
	std::string& prepend(std::string& _s, const std::string _pr) {
		return _s.insert(0, _pr);
	}

	/**
	* \brief Concatenates two std::string instances into a single std::string.
	*
	* \param _lhs First instance of std::string.
	* \param _rhs Second instance of std::string.
	* \return A std::string instance containing concatenated _lhs and _rhs.
	*/
	std::string concat(const std::string& _lhs, const std::string& _rhs) {
		return _lhs + _rhs;
	}

	/**
	* \brief Determines whether a std::string instance starts with a given char.
	*
	* \param _s Instance of std::string.
	* \param _c Character to check against.
	* \return True if _s starts with _c, false otherwise.
	*/
	bool starts_with(const std::string& _s, char _c) {
		return *_s.cbegin() == _c;
	}

	/**
	* \brief Determines whether a std::string instance ends with a given char.
	*
	* \param _s Instance of std::string.
	* \param _c Character to check against.
	* \return True if _s ends with _c, false otherwise.
	*/
	bool ends_with(const std::string& _s, char _c) {
		return *--(_s.cend()) == _c;
	}

	/**
	* \brief Converts all char's in a std::string to upper case.
	*
	* \param _s Reference to std::string instance.
	* \return Reference to _s with all char's converted to upper case.
	*/
	std::string& to_upper(std::string& _s) {
		for (auto& c : _s)
			c = static_cast<char>(std::toupper(c));
		return _s;
	}

	/**
	* \brief Converts all char's in a std::string to lower case.
	*
	* \param _s Reference to std::string instance.
	* \return Reference to _s with all char's converted to lower case.
	*/
	std::string& to_lower(std::string& _s) {
		for (auto& c : _s)
			c = static_cast<char>(std::tolower(c));
		return _s;
	}

	/**
	* \brief Removes all whitespaces from a std::string.
	*
	* \param _s Reference to std::string instance.
	* \return Reference to _s with all whitespace char's removed.
	*/
	std::string& trim(std::string& _s) {
		// invoke erase-remove idiom and use std::isspace(char) to check for whitespaces.
		_s.erase(std::remove_if(_s.begin(), _s.end(), [](char x) {return std::isspace(x); }), _s.end());
		return _s;
	}

	/**
	* \brief Removes all instances of any char in a given std::set<char> from a std::string.
	*
	* \param _s Reference to std::string instance.
	* \param _char_set A std::set containing instances of char to remove from _s.
	* \return Reference to _s with all instances of any char within _char_set removed.
	*/
	std::string& trim(std::string& _s, const std::set<char>& _char_set) {
		// invoke erase-remove idiom and pass _char_set to lambda captue for checking if char's occur
		_s.erase(std::remove_if(_s.begin(), _s.end(), [&_char_set](char x) {return _char_set.find(x) != _char_set.end(); }), _s.end());
		return _s;
	}

	/**
	* \brief Removes all vowel characters from a std::string.
	*
	* \param _s Reference to std::string instance.
	* \param Reference to _s with all vowels removed.
	*/
	std::string& remove_vowels(std::string& _s) {
		std::set<char> vowel_set = { 'a', 'e', 'i', 'o', 'u', 'A', 'E', 'I', 'O', 'U' };
		return trim(_s, vowel_set);
	}

	/**
	* \brief Removes all leading whitespace char's from a std::string instance.
	*
	* \param _s Reference to std::string instance.
	* \return Reference to _s with all leading whitespaces removed.
	*/
	std::string& remove_leading_whitespaces(std::string& _s) {
		auto it = _s.begin();
		// loop until past-the-end iterator of _s
		while (it != _s.end()) {
			// if character is a space, erase it and set it to
			// next valid std::string::iterator in _s
			if (std::isspace(*it)) {
				it = _s.erase(it);
			}
			// if character is not a space, finished so break
			else
				break;
		}
		return _s;
		// alternatively: 
		//_s.erase(0, _s.find_first_not_of(' '));
		//return _s;
	}

	/**
	* \brief Removes all trailing whitespace char's from a std::string instance.
	*
	* \param _s Reference to std::string instance.
	* \return Reference to _s with all trailing whitespaces removed.
	*/
	std::string& remove_trailing_whitespaces(std::string& _s) {
		auto it = _s.rbegin();
		// loop until rend iterator of _s
		while (it != _s.rend()) {
			// if character is a space, increment the iterator,
			// erase the space and set it to next valid reverse
			// iterator of std::string _s 
			if (std::isspace(*it)) {
				++it;
				it = std::reverse_iterator<std::string::iterator>(_s.erase(it.base()));
			}
			// if character is not a space, finished so break
			else
				break;
		}
		return _s;
		// alternatively:
		//_s.erase(_s.find_last_not_of(' '));
		//return _s;
	}

}

#endif