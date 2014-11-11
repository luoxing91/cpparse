#pragma once

#include <memory>

namespace cpparse
{
namespace detail
{
	//! Standardize the method of getting parser information.
	/*! "type_pointer" no longer has to be defined in every parser. */
	template<class P>
	class parser_traits
	{
	public:
		typedef typename P::value_type value_type;
		typedef typename P::result_type result_type;
		
		typedef std::shared_ptr<P> type_pointer;

	public:
		parser_traits() = delete;
		parser_traits(const parser_traits&) = delete;
		~parser_traits() = delete;
	};
}
}
