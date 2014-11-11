#pragma once

#include <string>
#include <memory>

#include "parser.h"
#include "../maybe.h"
#include "../buffer.h"

namespace cpparse
{
namespace detail
{
	//! Parse a sequence of characters in order.
	class string_parser : public parser<std::string, std::string>
	{
	public:
		string_parser(const std::string& s)
		: parser<std::string, std::string>(), m_string(s) {}

		string_parser(const string_parser&) = delete;
		~string_parser() = default;

		maybe<std::string> parse(buffer<std::string>& buffer) const
		{
			auto start = buffer.here();

			for (auto& ch : m_string)
			{
				auto next = buffer.next();
				if (next.is_just() && next.from_just() == ch)
					continue;

				buffer.rewind(start);
				return maybe<std::string>::nothing;
			}

			return maybe<std::string>::just(m_string);
		}

	private:
		std::string m_string;
	};

	//! Parse a single character.
	/*! Note that this parser returns a character, not a string. */
	class char_parser : public parser<char, std::string>
	{
	public:
		char_parser(char c)
		: parser<char, std::string>(), m_char(c) {}

		char_parser(const char_parser&) = delete;
		~char_parser() = default;

		maybe<char> parse(buffer<std::string>& buffer) const
		{
			auto start = buffer.here();

			auto next = buffer.next();
			if (next.is_just() && next.from_just() == m_char)
				return maybe<char>::just(m_char);

			buffer.rewind(start);
			return maybe<char>::nothing;
		}

	private:
		char m_char;
	};
}
}
