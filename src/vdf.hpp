#pragma once

#include <variant>
#include <memory> // shared_ptr
#include <vector>
#include <string>
#include <stdexcept> // runtime_error


class VDF
{
public:  // KeyValue definitions //

	struct KeyValue
	{
		std::string key = "";
		std::variant<std::shared_ptr<VDF>, std::string> val = "";

		// Construct empty KeyValue pair. (`key` = `""`, `val` = `nullptr`)
		KeyValue() = default;

		// Construct a KeyValue pair from string and VDF pointer.
		// Using a nullptr is undefined behaviour.
		KeyValue(const std::string & key, const std::shared_ptr<VDF> & val);

		// Construct a KeyValue pair from two strings
		KeyValue(const std::string & key, const std::string & val);

		// Construct a new KeyValue pair from another. `val` points to the same VDF object. This is not a deep copy!
		KeyValue(const KeyValue & other) = default;

		// Assign left-side KeyValue to be identical to the right-side KeyValue. `val` points to the same VDF object. This is not a deep copy!
		KeyValue & operator =(const KeyValue & other) = default;

		// Destructor.
		~KeyValue() = default;
	};

	friend std::ostream & operator <<(std::ostream & os, const KeyValue & t);

private:  // member variables //

	// stores all the actual data
	std::vector<KeyValue> data;

public:  // basic class API //

	// Construct empty VDF.
	VDF() = default;

	// Construct VDF from list of KeyValue pairs.
	VDF(const std::vector<KeyValue> & vec);

	// Construct new VDF from another. This is not a deep copy!
	VDF(const VDF & other) = default;

	// Assign left-side VDF to be identical to the right-side VDF. This is not a deep copy!
	VDF & operator =(const VDF & other) = default;

	VDF(VDF &&) = default;

	// Destructor.
	~VDF() = default;

	// Iterator stuff to allow range-based for loops.

	auto begin() { return data.begin(); }
	auto end() { return data.end(); }

	class TokenizationException : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error; // use parent constructor
	};

	class ParsingException : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error; // use parent constructor
	};

private:  // Parsing/Serializing //

	struct Token
	{
		enum {Undefined, String, Comment, OpenBrace, CloseBrace, End} type = Undefined;
		std::variant<std::string, int> data = -985959875;
	};

	friend std::ostream & operator <<(std::ostream & os, const Token & t);

	// The first step in parsing a string representation of a VDF.
	static std::vector<Token> tokenize(const std::string & vdfstring);

	// The second step in parsing a string representation of a VDF. This function recursivly calls itself, so it can't be merged with `tokenize()`.
	// Throws if the input is invalid.
	static VDF parse_tokens(const std::vector<Token> & tokens, int depth);

	// Serialize this VDF into a string. The result can be directly fed into a file.
	std::string serialize(int depth);

public:  // API for Parsing/Serializing //

	// Reads a string and turns it into a new VDF object.
	// May throw exceptions. (File reading errors or malformed VDF text)
	static VDF parse_from_string(const std::string & vdfstring);

	// Reads the file at the specified path and turns it into a new VDF object.
	// May throw exceptions. (Malformed VDF text)
	static VDF parse_from_filepath(const std::string & filepath);

	// Writes this VDF object as a human readable string.
	std::string serialize_to_string();

	// Writes this VDF object to a human readable file.
	// May throw exceptions. (File writing/creation errors)
	void serialize_to_filepath(const std::string & filepath);

private:  // misc //

	// Internal version of print_debug_info().
	void print_debug_recursive(int depth);

public:  // misc //

	// Prints detailed information about the VDF object and its data to the console.
	void print_debug_info();
};
