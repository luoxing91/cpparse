#pragma once

#include <type_traits>

#include "parser.h"
#include "detail/combinator.h"
#include "detail/parser_traits.h"

namespace cpparse
{
	// ******************************************************************
	//! Choice Combinator - choose output from one of two parsers.
	// ******************************************************************
	template<typename R, typename T>
	using choice_combinator = typename detail::parser_traits<detail::choice_combinator<R, T>>::type_pointer;

	template<class P>
	choice_combinator<out_type<P>, in_type<P>> operator|(P a, parser<out_type<P>, in_type<P>> b)
	{
		return make_parser<choice_combinator<out_type<P>, in_type<P>>>(a, b);
	}

	// ******************************************************************
	//! Sequence Combinator - use two parsers in a row if both succeed.
	// ******************************************************************
	template<typename R, typename T, typename M>
	using sequence_combinator = typename detail::parser_traits<detail::sequence_combinator<R, T, M>>::type_pointer;

	//! For this combinator, "in_type<P>" must equal "in_type<Q>".
	template<class P, class Q>
	sequence_combinator<out_type<Q>, in_type<P>, out_type<P>> operator>>(P a, Q b)
	{
		return make_parser<sequence_combinator<out_type<Q>, in_type<P>, out_type<P>>>(a, b);
	}

	// ******************************************************************
	//! Merge Combinator - combine the output of two parsers.
	// ******************************************************************
	template<typename R, typename T, template<typename> class A>
	using merge_combinator = typename detail::parser_traits<detail::merge_combinator<R, T, A>>::type_pointer;

	/*! If the vector accumulator is used here, note that chained calls will create nested vectors
	 *  of only two elements. As in [a, [b, [c, d]]] for three calls. FIXME: Modify vector accumulator
	 *  to merge passed arrays instead of nesting them?
	 */
	template<template<typename> class A, class P>
	merge_combinator<out_type<P>, in_type<P>, A> operator>>=(P a, parser<out_type<P>, in_type<P>> b)
	{
		return make_parser<merge_combinator<out_type<P>, in_type<P>, A>>(a, b);
	}

	// ******************************************************************
	//! Many Combinator - repeatedly use a parser until it fails.
	// ******************************************************************
	template<typename R, typename T, template<typename> class A>
	using many_combinator = typename detail::parser_traits<detail::many_combinator<R, T, A>>::type_pointer;

	/*! The accumulator for many and many1 should default to the vector version, however once A no
	 *  longer needs to be explicitly given, the char parser overrides are no longer chosen over
	 *  this default version with P. FIXME: change name of char many / figure out new template format.
	 */
	template<template<typename> class A, class P>
	many_combinator<out_type<P>, in_type<P>, A> many(P p, std::size_t min = 0, std::size_t max = 0)
	{
		return make_parser<many_combinator<out_type<P>, in_type<P>, A>>(p, min, max);
	}

	template<template<typename> class A, class P>
	many_combinator<out_type<P>, in_type<P>, A> many1(P p, std::size_t max = 0)
	{
		return many<A, P>(p, 1, max);
	}

	// ******************************************************************
	//! Block Combinator - perform a series of parser actions.
	// ******************************************************************
	template<typename R, typename T, typename M>
	using block_combinator = typename detail::parser_traits<detail::block_combinator<R, T, M>>::type_pointer;

	//! All three template arguments must be specified for this function.
	template<typename R, typename T, typename M>
	block_combinator<R, T, M> block()
	{
		return make_parser<block_combinator<R, T, M>>();
	}

	/*! This group of functions is necessary because the block parser ->* operator can take
	 *  a parser of any type, but it will only lift parsers that do not match its M type.
	 */
	template<class B, class P>
	struct block_converter
	{
	public:
		static typename B::element_type::element_pointer convert(P p)
		{
			return convert_impl(p, std::is_same<out_type<P>, out_type<typename B::element_type::element_pointer>>());
		}

	private:
		static typename B::element_type::element_pointer convert_impl(P p, std::true_type) { return p; }
		static typename B::element_type::element_pointer convert_impl(P p, std::false_type)
		{
			auto make_type = [](const out_type<P>& v) { return out_type<typename B::element_type::element_pointer>(); };
			auto lifted = lift<out_type<typename B::element_type::element_pointer>>(p, make_type);

			return lifted;
		}
	};

	template<class B, class P>
	B operator->*(B b, P p)
	{
		auto uniform = block_converter<B, P>::convert(p);

		b->then(uniform);
		return b;
	}

	//! Basically running out of operators here.
	/*! Kind of looks like Objective-C's block syntax though. */
	template<class B, typename F>
	B operator^(B b, const F& f)
	{
		b->evaluate(f);
		return b;
	}
}
