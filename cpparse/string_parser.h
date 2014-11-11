#pragma once

#include <string>

#include "parser.h"
#include "detail/string_parser.h"
#include "detail/parser_traits.h"

namespace cpparse
{
	// ******************************************************************
	//! Char Parser - parse a single character.
	// ******************************************************************
	using char_parser = typename detail::parser_traits<detail::char_parser>::type_pointer;

	char_parser character(char c)
	{
		return make_parser<char_parser>(c);
	}

	// ******************************************************************
	//! Char Overrides - specialize templates for char parsers.
	// ******************************************************************

	using oneof_char_parser = oneof_parser<char, std::string>;
	oneof_char_parser one_of(const std::string& s)
	{
		return one_of<std::string>(std::vector<char>(s.begin(), s.end()));
	}

	using noneof_char_parser = noneof_parser<char, std::string>;
	noneof_char_parser none_of(const std::string& s)
	{
		return none_of<std::string>(std::vector<char>(s.begin(), s.end()));
	}

	// ******************************************************************
	//! String parser - parse a sequence of characters.
	// ******************************************************************
	using string_parser = typename detail::parser_traits<detail::string_parser>::type_pointer;

	string_parser string(const std::string& s)
	{
		return make_parser<string_parser>(s);
	}
}
