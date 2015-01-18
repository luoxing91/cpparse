#pragma once

#include <vector>
#include <functional>

#include "detail/parser.h"
#include "detail/parser_traits.h"

namespace cpparse
{
	// ******************************************************************
	//! Parser - all parsers derive from this.
	// ******************************************************************
	template<typename R, typename T>
	using parser = typename detail::parser_traits<detail::parser<R, T>>::type_pointer;

	//! Some utilities to make transferring type info easier.
	template<class P>
	using in_type = typename detail::parser_traits<typename P::element_type>::value_type;
	template<class P>
	using out_type = typename detail::parser_traits<typename P::element_type>::result_type;

	//! Create a shared_ptr to a new parser P.
	template<class P, typename... Args>
	P make_parser(Args&&... args)
	{
		return std::make_shared<typename P::element_type>(std::forward<Args>(args)...);
	}

	//! Cast a pointer to a base parser to its derived class.
	template<class P>
	P parser_cast(parser<out_type<P>, in_type<P>> p)
	{
		return std::dynamic_pointer_cast<typename P::element_type>(p);
	}

	//! Define a "tag" so some parsers can be referenced by name.
	/*! This is mostly useful for the block parser, since it records results based on tag values. */
	struct parser_tag { std::string string; };
	parser_tag tag(const std::string& s) { return {s}; }

	template<class P>
	P operator<<(const P& p, const parser_tag& t)
	{
		auto tagged = make_parser<P>(*p);
		tagged->set_tag(t.string);

		return tagged;
	}

	// ******************************************************************
	//! Forward Parser - wraps another parser; good for recursion.
	// ******************************************************************
	template<typename R, typename T>
	using forward_parser = typename detail::parser_traits<detail::forward_parser<R, T>>::type_pointer;

	//! The template arguments always have to be specified for this function.
	template<typename R, typename T>
	forward_parser<R, T> placeholder()
	{
		return make_parser<forward_parser<R, T>>();
	}

	// ******************************************************************
	//! Skip Parser - the output of the inner parser is ignored.
	// ******************************************************************
	template<typename T, typename M>
	using skip_parser = typename detail::parser_traits<detail::skip_parser<T, M>>::type_pointer;

	/* All template utility function have to define the input parser type with a template class
	 * instead of typename R, typename T - the compiler is otherwise unable to infer the type
	 * since it is wrapped in a shared_ptr that cannot be implicitly converted from a container
	 * for the dervied class to a container for the base class.
	 */
	template<class P>
	skip_parser<in_type<P>, out_type<P>> skip(P p)
	{
		return make_parser<skip_parser<in_type<P>, out_type<P>>>(p);
	}

	// ******************************************************************
	//! Option Parser - provide alternative output upon parser failure.
	// ******************************************************************
	template<typename R, typename T>
	using option_parser = typename detail::parser_traits<detail::option_parser<R, T>>::type_pointer;

	template<class P>
	option_parser<out_type<P>, in_type<P>> option(P p, const out_type<P>& a)
	{
		return make_parser<option_parser<out_type<P>, in_type<P>>>(p, a);
	}

	template<class P>
	option_parser<out_type<P>, in_type<P>> optional(P p)
	{
		return option(p, out_type<P>());
	}

	// ******************************************************************
	//! Lift Parser - map the result of a parser to a new type.
	// ******************************************************************
	template<typename R, typename T, typename M>
	using lift_parser = typename detail::parser_traits<detail::lift_parser<R, T, M>>::type_pointer;

	//! The R parameter must always be specified, but the others will be deduced.
	template<typename R, typename F, class P>
	lift_parser<R, in_type<P>, out_type<P>> lift(P p, const F& f)
	{
		return make_parser<lift_parser<R, in_type<P>, out_type<P>>>(p, f);
	}

	// ******************************************************************
	//! OneOf Parser - match any one of a list of values to the input.
	// ******************************************************************
	template<typename R, typename T>
	using oneof_parser = typename detail::parser_traits<detail::oneof_parser<R, T>>::type_pointer;

	template<typename T, typename R>
	oneof_parser<R, T> one_of(const std::vector<R>& c)
	{
		return make_parser<oneof_parser<R, T>>(c);
	}

	// ******************************************************************
	//! NoneOf Parser - match any token except those in a list of values.
	// ******************************************************************
	template<typename R, typename T>
	using noneof_parser = typename detail::parser_traits<detail::noneof_parser<R, T>>::type_pointer;

	template<typename T, typename R>
	noneof_parser<R, T> none_of(const std::vector<R>& r)
	{
		return make_parser<noneof_parser<R, T>>(r);
	}
}
