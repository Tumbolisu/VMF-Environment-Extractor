#pragma once

#include <variant>
#include <memory> // shared_ptr
#include <vector>
#include <string>
#include <stdexcept> // runtime_error
#include <unordered_set>


class VDF
{
public:  // KeyValue definitions //

	struct KeyValue
	{
		// Key string.
		std::string key = "";
		// Value. Can be either a `string` or a nested VDF via a `shared_ptr<VDF>`.
		std::variant<std::shared_ptr<VDF>, std::string> val = "";

		// Construct empty KeyValue pair. (`key` = `""`, `val` = `nullptr`)
		KeyValue() = default;

		// Construct a KeyValue pair from string and VDF pointer.
		// Using a nullptr is undefined behaviour.
		KeyValue(const std::string & key, const std::shared_ptr<VDF> & val)
		: key(key)
		, val(val)
		{}

		// Construct a KeyValue pair from two strings
		KeyValue(const std::string & key, const std::string & val)
		: key(key)
		, val(val)
		{}

		// Construct a new KeyValue pair from another. `val` points to the same VDF object. This is not a deep copy!
		KeyValue(const KeyValue & other) = default;

		// Assign left-side KeyValue to be identical to the right-side KeyValue. `val` points to the same VDF object. This is not a deep copy!
		KeyValue & operator=(const KeyValue & other) = default;

		// Destructor.
		~KeyValue() = default;

		// Returns `true` if this KeyValue is empty, i.e. both key and value are an empty string.
		[[nodiscard]] bool empty() const noexcept;

		// Clears both key and value, resetting them to the default state of two empty strings.
		void clear() noexcept;
	};

	friend std::ostream & operator<<(std::ostream & os, const KeyValue & t);

private:  // member variables //

	// stores all the actual data
	std::vector<KeyValue> data;

public:  // basic class API //

	// Construct empty VDF.
	VDF() = default;

	// Construct VDF from list of KeyValue pairs.
	VDF(const std::vector<KeyValue> & vec)
	: data(vec)
	{}

	// Construct new VDF from another. This is not a deep copy!
	VDF(const VDF & other) = default;

	// Assign left-side VDF to be identical to the right-side VDF. This is not a deep copy!
	VDF & operator=(const VDF & other) = default;

	// Move Constructor.
	VDF(VDF &&) = default;

	// Destructor.
	~VDF() = default;

	// Iterator stuff to allow range-based for loops.

	inline auto cbegin() const noexcept { return data.cbegin(); }
	inline auto cend()   const noexcept { return data.cend();   }
	inline auto begin() const noexcept { return cbegin(); }
	inline auto end()   const noexcept { return cend();   }
	inline auto begin() noexcept { return data.begin(); }
	inline auto end()   noexcept { return data.end();   }

	// Returns a list of every KeyValue with matching `key`.
	// The elements are raw pointers, which allows you to directly edit the VDF contents.
	std::vector<KeyValue *> find_all(const std::string & key);

	// Returns a list of every KeyValue with matching `key`.
	// `[const qualified]` The elements are read-only raw pointers.
	const std::vector<const KeyValue *> find_all(const std::string & key) const;

	// Returns `true` if `a` and `b` are equal, meaning they contain the same list of keys with the same values, all in the same order.
	// Keys listed in `ignore_keys` are ignored.
	// If `ignore_order` is `true`, the order of keys is ignored.
	[[nodiscard]] static bool compare(
			const VDF & a,
			const VDF & b,
			const std::unordered_set<std::string> & ignore_keys,
			bool ignore_order) noexcept;

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

	friend std::ostream & operator<<(std::ostream & os, const Token & t);

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
