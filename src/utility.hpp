#pragma once

#include <string>
#include <set>
#include <unordered_set>
#include <map>
#include <typeinfo>


// Performs a linear search on a string to find an ASCII whitespace character.
// Returns the position of the first whitespace character if one exists,
// returns `std::string::npos` otherwise.
// The ASCII whitespace characters are `'\\t'`, `'\\n'`, `'\\v'`, `'\\f'`, `'\\r'` and `' '`.
[[nodiscard]] std::size_t find_whitespace(const std::string & s);

// Performs a linear search on a string to find an ASCII whitespace character.
// Returns `true` if a whitespace character exists,
// returns `false` otherwise.
// The ASCII whitespace characters are `'\\t'`, `'\\n'`, `'\\v'`, `'\\f'`, `'\\r'` and `' '`.
[[nodiscard]] bool has_whitespace(const std::string & s);


// Checks if string `a` ends with string `b`.
[[nodiscard]] inline bool string_ends_with(const std::string & a, const std::string & b)
{
	return (a.length() >= b.length()) && (a.compare(a.length() - b.length(), b.length(), b) == 0);
}


// Checks if the set `set` containts the key `key`.
template<class Key, class Compare, class Allocator>
[[nodiscard]] inline bool contains(const std::set<Key,Compare,Allocator> & set, Key key)
{
	auto search = set.find(key);
	return search != set.end();
}


// Checks if the unordered set `set` containts the key `key`.
template<class Key, class Compare, class Allocator>
[[nodiscard]] inline bool contains(const std::unordered_set<Key,Compare,Allocator> & set, Key key)
{
	auto search = set.find(key);
	return search != set.end();
}


// Checks if the map `map` containts the key `key`.
template<class Key, class T, class Compare, class Allocator>
[[nodiscard]] inline bool contains(const std::map<Key,T,Compare,Allocator> & map, Key key)
{
	auto search = map.find(key);
	return search != map.end();
}


// Demangles the compiler-specific name of a class. Useful for making error messages!
std::string demangle(const char * name);

// Returns the name of the class of the input object.
template <class T>
inline std::string type(const T & t)
{
	return demangle(typeid(t).name());
}
