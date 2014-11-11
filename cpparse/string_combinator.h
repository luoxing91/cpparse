#pragma once

#include <string>

#include "combinator.h"
#include "string_parser.h"
#include "accumulator.h"
#include "detail/string_parser.h"
#include "detail/parser_traits.h"

namespace cpparse
{
	// ******************************************************************
	//! Char Overrides - specialize templates for char combinators.
	// ******************************************************************

	using many_char_combinator = many_combinator<char, std::string, string_accumulator>;

	//! These overrides only work because the base "many" function requires an explicit template argument.
	many_char_combinator many(parser<char, std::string> p, std::size_t min = 0, std::size_t max = 0)
	{
		return many<string_accumulator>(p, min, max);
	}

	many_char_combinator many1(parser<char, std::string> p, std::size_t max = 0)
	{
		return many1<string_accumulator>(p, max);
	}

	// ******************************************************************
	//! String Overrides - specialize templates for string combinators.
	// ******************************************************************

	using merge_string_combinator = merge_combinator<std::string, std::string, concat_accumulator>;

	//! The workaround for the "many" specializations applies here as well.
	merge_string_combinator operator>>=(parser<std::string, std::string> a, parser<std::string, std::string> b)
	{
		return operator>>=<concat_accumulator>(a, b);
	}
}
