#pragma once

#include <string>
#include <vector>
#include <functional>
#include <algorithm>

#include "../maybe.h"
#include "../buffer.h"
#include "parser_traits.h"

namespace cpparse
{
namespace detail
{
	//! The generic parser interface.
	/*! Takes an input type and an output type. Note that the actual type
	 *  to be parsed is "buffer<T>::value_type".
	 */
	template<typename R, typename T>
	class parser
	{
	public:
		typedef T value_type;
		typedef R result_type;

	public:
		parser() = default;
		parser(const parser& other) = default;
		virtual ~parser() = default;

		const std::string& tag() const { return m_tag; }
		std::string tag() { return m_tag; }

		void set_tag(const std::string& t) { m_tag = t; }

		//! All parser types should overload this function.
		/*! A parser is also expected to, upon failure, return the buffer to
		 *  its state at the start of the call.
		 */
		virtual maybe<result_type> parse(buffer<value_type>&) const = 0;

	private:
		std::string m_tag;
	};

	//! A basic parser 'wrapper'.
	/*! Takes another parser as the "target" and simply passes the parse
	 *  operation through, return the result of the "target". This allows for
	 *  lazy or recursive evaluation by acting as a placeholder.
	 */
	template<typename R, typename T>
	class forward_parser : public parser<R, T>
	{
	private:
		typedef typename parser_traits<parser<R, T>>::type_pointer subtype_pointer;

	public:
		//! For normal use, the "target" will likely never be ready upon creation.
		forward_parser()
		: parser<R, T>(), m_target(nullptr) {}

		forward_parser(const forward_parser&) = default;
		~forward_parser() = default;

		//! Set the parser to pass operations to. Can also be reset to nullptr.
		void set_target(subtype_pointer p) { m_target = p; }

		maybe<R> parse(buffer<T>& buffer) const
		{
			maybe<R> result = m_target->parse(buffer);
			return result;
		}

	private:
		subtype_pointer m_target;
	};

	//! A parser whose output is ignored.
	/*! The actual value of the inner parser is discared and an empty value is returned. 
	 *  I think this is more useful than void, since any output can be ignored, but
	 *  an empty value allows skip to still be chained. (i.e., (a parser) >>= skip >>= (a parser) )
	 */
	template<typename T, typename M>
	class skip_parser : public parser<M, T>
	{
	private:
		typedef typename parser_traits<parser<M, T>>::type_pointer subtype_pointer;

	public:
		skip_parser(subtype_pointer p)
		: parser<M, T>(), m_parser(p) {}

		skip_parser(const skip_parser&) = default;
		~skip_parser() = default;

		maybe<M> parse(buffer<T>& buffer) const
		{
			auto ignore = m_parser->parse(buffer);
			if (ignore.is_just())
				return maybe<M>::just(M());

			return maybe<M>::nothing;
		}

	private:
		subtype_pointer m_parser;
	};

	//! A parser that, on failure, returns an alternate value.
	/*! The option parser always returns maybe<R>::just. */
	template<typename R, typename T>
	class option_parser : public parser<R, T>
	{
	private:
		typedef typename parser_traits<parser<R, T>>::type_pointer subtype_pointer;

	public:
		option_parser(subtype_pointer p, const R& a)
		: parser<R, T>(), m_parser(p), m_alternate(a) {}

		option_parser(const option_parser&) = default;
		~option_parser() = default;

		maybe<R> parse(buffer<T>& buffer) const
		{
			auto possible = m_parser->parse(buffer);

			R result = (possible.is_just()) ? possible.from_just() : m_alternate;
			return maybe<R>::just(result);
		}

	private:
		subtype_pointer m_parser;
		R m_alternate;
	};

	//! A parser to "lift" values, i.e. convert them to a more general type.
	/*! The lift parser takes a parser of type T->M and wraps it, using a function
	 *  pointer to convert the result type M->R. This allows a parser of one type to
	 *  interact with a parser of another type, mapping T->M->R.
	 */
	template<typename R, typename T, typename M>
	class lift_parser : public parser<R, T>
	{
	private:
		typedef typename parser_traits<parser<M, T>>::type_pointer subtype_pointer;

	public:
		//! The constructor can take a normal function pointer or a lambda.
		template<typename F>
		lift_parser(subtype_pointer p, const F& f)
		: parser<R, T>(), m_parser(p), m_function(f) {}

		lift_parser(const lift_parser&) = default;
		~lift_parser() = default;

		maybe<R> parse(buffer<T>& buffer) const
		{
			auto to_lift = m_parser->parse(buffer);
			if (to_lift.is_nothing())
				return maybe<R>::nothing;

			auto lifted = m_function(to_lift.from_just());
			return maybe<R>::just(lifted);
		}

	private:
		subtype_pointer m_parser;
		//! The supplied function is passed a reference to the parsed result.
		std::function<R(const M&)> m_function;
	};

	//! Attempts to match a token with any value in an array.
	/*! Tries to cast buffer<T>::value_type into R as the return type. */
	template<typename R, typename T>
	class oneof_parser : public parser<R, T>
	{
	public:
		oneof_parser(const std::vector<R>& c)
		: parser<R, T>(), m_choices(c) {}

		oneof_parser(const oneof_parser&) = default;
		~oneof_parser() = default;

		maybe<R> parse(buffer<T>& buffer) const
		{
			auto start = buffer.here();

			auto next = buffer.next();
			if (next.is_nothing())
				return maybe<R>::nothing;

			R result(next.from_just());

			auto pos = std::find(m_choices.begin(), m_choices.end(), result);
			if (pos != m_choices.end())
				return maybe<R>::just(next.from_just());

			buffer.rewind(start);
			return maybe<R>::nothing;
		}

	private:
		std::vector<R> m_choices;
	};

	//! Attempts to match a token with any value but the ones in an array.
	/*! Uses the same element and return type as "oneof_parser". */
	template<typename R, typename T>
	class noneof_parser : public parser<R, T>
	{
	public:
		noneof_parser(const std::vector<R>& c)
		: parser<R, T>(), m_rejects(c) {}

		noneof_parser(const noneof_parser&) = default;
		~noneof_parser() = default;

		maybe<R> parse(buffer<T>& buffer) const
		{
			auto start = buffer.here();

			auto next = buffer.next();
			if (next.is_nothing())
				return maybe<R>::nothing;

			R result(next.from_just());

			auto pos = std::find(m_rejects.begin(), m_rejects.end(), result);
			if (pos == m_rejects.end())
				return maybe<R>::just(next.from_just());

			buffer.rewind(start);
			return maybe<R>::nothing;
		}

	private:
		std::vector<R> m_rejects;
	};
}
}
