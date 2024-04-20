#include "vdf.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>

#include "utility.hpp"


//// VDF::KeyValue ////


[[nodiscard]] bool VDF::KeyValue::empty() const noexcept
{
	return (key.empty())
	&&     (std::holds_alternative<std::string>(val))
	&&     (std::get<std::string>(val).empty());
}

void VDF::KeyValue::clear() noexcept
{
	key.clear();
	val = "";
}


std::ostream & operator<<(std::ostream & os, const VDF::KeyValue & kv)
{
	using namespace std;
	os << "KV(\"" << kv.key << "\",";
	if (holds_alternative<string>(kv.val))
	{
		os << "\"" << get<string>(kv.val) << "\")";
	}
	else
	{
		os << "<VDF>@" << get<shared_ptr<VDF>>(kv.val) << ")";
	}
	return os;
}


//// VDF::Token ////


std::ostream & operator<<(std::ostream & os, const VDF::Token & t)
{
	using namespace std;
	switch (t.type)
	{
	case VDF::Token::Undefined:
		os << "T(Undefined)"; break;
	case VDF::Token::String:
		os << "T(String:\"" << get<string>(t.data) << "\")"; break;
	case VDF::Token::Comment:
		os << "T(Comment:\"" << get<string>(t.data) << "\")"; break;
	case VDF::Token::OpenBrace:
		os << "T(OpenBrace:" << get<int>(t.data) << ")"; break;
	case VDF::Token::CloseBrace:
		os << "T(CloseBrace:" << get<int>(t.data) << ")"; break;
	case VDF::Token::End:
		os << "T(End)"; break;
	default:
		os << "T(Unknown Token Type!)"; break;
	}
	return os;
}


//// VDF ////

std::vector<VDF::KeyValue *> VDF::find_all(const std::string & key)
{
	using namespace std;
	std::vector<VDF::KeyValue *> result;
	for (KeyValue & kv : data)
	{
		if (kv.key == key)
		{ result.push_back(&kv); }
	}
	return result;
}

const std::vector<const VDF::KeyValue *> VDF::find_all(const std::string & key) const
{
	using namespace std;
	std::vector<const VDF::KeyValue *> result;
	for (const KeyValue & kv : data)
	{
		if (kv.key == key)
		{ result.push_back(&kv); }
	}
	return result;
}

[[nodiscard]] bool VDF::compare(
		const VDF & a,
		const VDF & b,
		const std::unordered_set<std::string> & ignore_keys,
		bool ignore_order) noexcept
{
	using namespace std;

	if (ignore_order)
	{
		// Loop through every KeyValue in `a` and try to pair it up with a KeyValue in `b`.
		// If a matching KeyValue can't be found, or if something is left unpaired from `b`, then the VDFs aren't equal.

		enum class State {Unpaired, PairedUp, Ignored};
		vector<State> states_a (a.data.size(), State::Unpaired);
		vector<State> states_b (b.data.size(), State::Unpaired);

		// First mark all ignored KeyValues.

		for (size_t i = 0; i < a.data.size(); ++i)
		{
			if (contains(ignore_keys, a.data[i].key))
			{ states_a[i] = State::Ignored; }
		}

		for (size_t i = 0; i < b.data.size(); ++i)
		{
			if (contains(ignore_keys, b.data[i].key))
			{ states_b[i] = State::Ignored; }
		}

		// Now do the actual pair-matching.

		for (size_t ia = 0; ia < a.data.size(); ++ia)
		{
			const auto & key_a = a.data[ia].key;
			const auto & val_a = a.data[ia].val;
			auto & state_a = states_a[ia];

			if (state_a == State::Ignored)
			{ continue; }

			// assert: state_a == State::Unpaired

			for (size_t ib = 0; ib < b.data.size(); ++ib)
			{
				const auto & key_b = b.data[ib].key;
				const auto & val_b = b.data[ib].val;
				auto & state_b = states_b[ib];

				if (state_b == State::Ignored
				||  state_b == State::PairedUp)
				{ continue; }

				if (key_a != key_b)
				{ continue; }

				if (holds_alternative<string>(val_a))
				{
					if (holds_alternative<string>(val_b)
					&&  get<string>(val_a) == get<string>(val_b))
					{
						state_a = state_b = State::PairedUp;
						break;
					}
				}
				else
				{
					if (holds_alternative<shared_ptr<VDF>>(val_b)
					&&  compare(*get<shared_ptr<VDF>>(val_a), *get<shared_ptr<VDF>>(val_b), ignore_keys, ignore_order))
					{
						state_a = state_b = State::PairedUp;
						break;
					}
				}
			}

			if (state_a == State::Unpaired)
			{ return false; }
		}

		for (const auto & s : states_b)
		{
			if (s == State::Unpaired)
			{ return false; }
		}

		return true;
	}
	else
	{
		// TODO: make this
		throw "VDF::compare() with ignore_order = false is not yet implemented!";
		return false;
	}
}


std::vector<VDF::Token> VDF::tokenize(const std::string & vdfstring)
{
	using namespace std;
	vector<Token> tokens;
	// The worst average number of characters per token is about 25.
	tokens.reserve(vdfstring.size() / 32);
	int brace_depth = 0;
	string::size_type i = 0; // index of current character
	string::size_type j = 0; // used for string::find()

	auto throw_error = [&tokens, &i, &brace_depth](const char * what) -> void
	{
		cerr << "---- ERROR ---- ERROR ---- ERROR ----" << endl;
		for (Token & token : tokens) cerr << token << endl;
		cerr << "i=" << i << ", brace_depth=" << brace_depth << endl;
		throw TokenizationException(what);
	};

	while (i < vdfstring.size())
	{
		switch(vdfstring[i])
		{
		case '\t': case '\n': case '\v': case '\f': case '\r': case ' ': // whitespace
		{
			i += 1;
			break;
		}
		case '{': // open brace
		{
			tokens.push_back(Token{Token::OpenBrace, brace_depth});
			brace_depth += 1;
			i += 1;
			break;
		}
		case '}': // close brace
		{
			brace_depth -= 1;
			if (brace_depth < 0)
			{ throw_error("Negative brace depth! (There are more closing braces than opening braces.)"); }
			tokens.push_back(Token{Token::CloseBrace, brace_depth});
			i += 1;
			break;
		}
		case '"': // string (delimited, may contain spaces)
		{
			j = vdfstring.find('"', i+1);
			if (j == vdfstring.npos)
			{ throw_error("String without closing quote!"); }
			tokens.push_back(Token{Token::String, vdfstring.substr(i+1, j-i-1)});
			i = j+1;
			break;
		}
		case '/': // possibly a comment. check next character
		{
			char next = (i+1 < vdfstring.size()) ? vdfstring[i+1] : '\0';
			if (next == '/') // actually a comment
			{
				j = vdfstring.find('\n', i+2);
				if (j == vdfstring.npos)  // newline not found, slice to end
				{
					tokens.push_back(Token{Token::Comment, vdfstring.substr(i+2)});
					i = j;
				}
				else
				{
					tokens.push_back(Token{Token::Comment, vdfstring.substr(i+2, j-i-2)});
					i = j+1;
				}
				break;
			}
			// not a comment? fall-through to string
		}
		default: // string (not delimited, can't contain spaces)
		{
			j = i+1;
			// find next whitespace character
			while (true)
			{
				switch(vdfstring[j])
				{
				case '\0': // end of string
					goto superbreak3774;
				case '\t': case '\n': case '\v': case '\f': case '\r': case ' ': // whitespace
					goto superbreak3774;
				default:
					j += 1;
				}
			}
			superbreak3774:
			tokens.push_back(Token{Token::String, vdfstring.substr(i, j-i)});
			i = j;
			break;
		}
		}
	}

	if (brace_depth > 0)
	{ throw_error("Positive brace depth! (There are more opening braces than closing braces.)"); }

	tokens.push_back(Token{Token::End, -1});

	return tokens;
}

VDF VDF::parse_tokens(const std::vector<VDF::Token> & tokens, int depth)
{
	using namespace std;
	VDF result;
	size_t i = 0;

	auto throw_error = [&tokens, &depth, &result, &i](const char * what) -> void
	{
		cerr << "---- ERROR ---- ERROR ---- ERROR ----" << endl;
		cerr << "tokens.size()=" << tokens.size() << endl;
		for (const Token & token : tokens) cerr << token << endl;
		cerr << "result.data.size()=" << result.data.size() << endl;
		for (KeyValue & kv : result.data) cerr << kv << endl;
		cerr << "i=" << i << ", tokens[i]=" << tokens[i] << endl;
		cerr << "depth=" << depth << endl;
		throw ParsingException(what);
	};

	while (i < tokens.size())
	{
		const Token & token = tokens[i];

		switch(token.type)
		{
		case VDF::Token::String:
		{
			size_t k = i;
			Token next_token;

			// Comments don't matter and need to be skipped over.
			do
			{
				k += 1;
				if (k >= tokens.size())
				{ throw_error("Key string can't pair with a value because there are no more tokens to parse."); }
				next_token = tokens[k];
			}
			while (next_token.type == VDF::Token::Comment);

			i = k;

			switch(next_token.type)
			{
			case VDF::Token::String:
			{
				result.data.push_back(KeyValue(get<string>(token.data), get<string>(next_token.data)));
				i += 1;
				break;
			}
			case VDF::Token::OpenBrace:
			{
				size_t j = i+1;
				bool found = false;
				const int target = get<int>(next_token.data);
				vector<Token> braced_tokens;

				while (j < tokens.size())
				{
					if (tokens[j].type == VDF::Token::CloseBrace
					&&  get<int>(tokens[j].data) == target)
					{
						found = true;
						break;
					}
					else
					{
						braced_tokens.push_back(tokens[j]);
						j += 1;
					}
				}

				if (!found)
				{ throw_error("Opening brace without matching closing brace. (This should have been caught by the tokenizer!)"); }

				braced_tokens.push_back(Token{Token::End, -1});

				try
				{
					shared_ptr<VDF> tempvdf{make_shared<VDF>(move(parse_tokens(braced_tokens, depth+1)))};
					result.data.push_back(KeyValue(get<string>(token.data), tempvdf));
				}
				catch (const ParsingException & ex)
				{
					throw_error(ex.what());
				}

				i = j+1;
				break;
			}
			default:
				throw_error("Key string is followed by nonsense and is therefore left without a value!");
			}

			break;
		}
		case VDF::Token::Comment:
		{
			// TODO comments are ignored and discarded.
			i += 1;
			break;
		}
		case VDF::Token::End:
		{
			i += 1;
			break;
		}
		default:
			throw_error("Unexpected token type!");
		}
	}

	return result;
}

std::string VDF::serialize(int depth)
{
	using namespace std;
	ostringstream result;
	const string tabs = string(depth, '\t');
	for (const KeyValue & kv : data)
	{
		if (holds_alternative<string>(kv.val))
		{
			if (kv.empty())
			{ continue; } // ignore empty lines

			result << tabs << "\"" << kv.key << "\" \"" << get<string>(kv.val) << "\"\n";
		}
		else
		{
			if (has_whitespace(kv.key))
			{
				result << tabs << "\"" << kv.key << "\"";
			}
			else
			{
				result << tabs << kv.key;
			}
			result << "\n" << tabs << "{\n"
			<< get<shared_ptr<VDF>>(kv.val)->serialize(depth+1)
			<< tabs << "}\n";
		}
	}
	return result.str();
}



VDF VDF::parse_from_string(const std::string & vdfstring)
{
	return parse_tokens(tokenize(vdfstring), 0);
}

VDF VDF::parse_from_filepath(const std::string & filepath)
{
	using namespace std;
	ostringstream filecontent;
	fstream infile {filepath, ifstream::in};
	infile.exceptions(ios_base::failbit);
	filecontent << infile.rdbuf();
	infile.close();
	return parse_from_string(filecontent.str());
}

std::string VDF::serialize_to_string()
{
	return serialize(0);
}

void VDF::serialize_to_filepath(const std::string & filepath)
{
	using namespace std;
	ofstream outfile {filepath};
	outfile.exceptions(ios_base::failbit);
	outfile << serialize_to_string();
	outfile.close();
}



void VDF::print_debug_recursive(int depth)
{
	using namespace std;
	const string tabs = string(depth*2, ' ');
	cout << tabs << "<VDF>@" << this << " BEGIN" << endl;
	cout << tabs << "(" << data.size() << " items)" << endl;
	for (size_t i = 0; i < data.size(); ++i)
	{
		KeyValue & kv = data[i];
		cout << tabs << "[" << i << "] " << "key=\"" << kv.key << "\"";
		if (holds_alternative<string>(kv.val))
		{
			cout << ", val=\"" << get<string>(kv.val) << "\"" << endl;
		}
		else
		{
			cout << ", val=" << endl;
			get<shared_ptr<VDF>>(kv.val)->print_debug_recursive(depth+1);
		}
	}
	cout << tabs << "<VDF>@" << this << " END" << endl;
}

void VDF::print_debug_info()
{
	VDF::print_debug_recursive(0);
}
