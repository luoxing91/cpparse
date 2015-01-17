#pragma once

#include <vector>
#include <string>

namespace cpparse
{
namespace detail
{
	template<typename R>
	struct join
	{
		typedef std::vector<R> result_type;
		static void append(result_type& res, const R& v) { res.push_back(v); }
	};

	template<>
	struct join<char>
	{
		typedef std::string result_type;
		static void append(result_type& res, const char& v) { res += v; }
	};

	template<>
	struct join<std::string>
	{
		typedef std::string result_type;
		static void append(result_type& res, const std::string& v) { res += v; }
	};
}
}
