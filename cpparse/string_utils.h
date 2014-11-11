#pragma once

#include "string_parser.h"
#include "string_combinator.h"

namespace cpparse
{
	//! Common character parsers.
	oneof_char_parser upper() { return one_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"); }
	oneof_char_parser lower() { return one_of("abcdefghijklmnopqrstuvwxyz"); }
	choice_combinator<char, std::string> letter() { return upper() | lower(); }

	oneof_char_parser digit() { return one_of("1234567890"); }
	oneof_char_parser symbol() { return one_of("!#$%&|*+-/:<=>?@^_~"); }

	//! Retrieve all whitespace between tokens.
	many_char_combinator spaces() { return many1(one_of(" \t\r\n")); }

	//! Convert a character parser to a string parser.
	lift_parser<std::string, std::string, char> lift_string(parser<char, std::string> p)
	{
		return lift<std::string>(p,
			[](char c)
			{
				return std::string(1, c);
			});
	}
}
