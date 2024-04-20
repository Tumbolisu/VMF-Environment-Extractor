#include "utility.hpp"


[[nodiscard]] std::size_t find_whitespace(const std::string & s)
{
	using namespace std;
	for (size_t i = 0; i < s.size(); ++i)
	{
		if ((s[i] >= '\t' && s[i] <= '\r') || (s[i] == ' '))
		{ return i; }
	}
	return string::npos;
}


[[nodiscard]] bool has_whitespace(const std::string & s)
{
	using namespace std;
	for (char c : s)
	{
		if ((c >= '\t' && c <= '\r') || (c == ' '))
		{ return true; }
	}
	return false;
}


#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
std::string demangle(const char * name)
{
	int status = 1;
	std::unique_ptr<char, void(*)(void*)> res {abi::__cxa_demangle(name, NULL, NULL, &status), std::free};
	return (status == 0) ? res.get() : name;
}
#else
// Does nothing if not g++
std::string demangle(const char * name)
{
	return name;
}
#endif