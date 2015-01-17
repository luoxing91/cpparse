#pragma once

#include <map>
#include <vector>
#include <functional>

#include "parser.h"
#include "../maybe.h"
#include "../buffer.h"
#include "parser_traits.h"

namespace cpparse
{
namespace detail
{
	//! The base interface for a basic combinator.
	/*! Represents the combination of two or more parsers that return the same type. */
	template<typename R, typename T, typename M>
	class uniform_combinator : public parser<R, T>
	{
	public:
		typedef typename parser_traits<parser<M, T>>::type_pointer element_pointer;

	public:
		uniform_combinator()
		: parser<R, T>() {}

		uniform_combinator(const uniform_combinator&) = delete;
		virtual ~uniform_combinator() = default;
	};

	//! Another combinator interface, allowing for more complex types.
	/*! Represents the combination of parsers with varying return types. */
	template<typename R, typename T, typename MP, typename MS>
	class composite_combinator : public parser<R, T>
	{
	public:
		typedef typename parser_traits<parser<MP, T>>::type_pointer major_pointer;
		typedef typename parser_traits<parser<MS, T>>::type_pointer minor_pointer;

	public:
		composite_combinator()
		: parser<R, T>() {}

		composite_combinator(const composite_combinator&) = delete;
		virtual ~composite_combinator() = default;
	};

	//! Attempt to match one of two parsers to the given input.
	/*! One parser will always be tried first, and upon failure, the second is tried.
	 *  If the second fails as well, "maybe<R>::nothing" is returned.
	 */
	template<typename R, typename T>
	class choice_combinator : public uniform_combinator<R, T, R>
	{
	public:
		typedef typename uniform_combinator<R, T, R>::element_pointer element_pointer;

	public:
		choice_combinator(element_pointer f, element_pointer s)
		: uniform_combinator<R, T, R>(), m_first(f), m_second(s) {}

		choice_combinator(const choice_combinator&) = delete;
		~choice_combinator() = default;

		maybe<R> parse(buffer<T>& buffer) const
		{
			auto first_result = m_first->parse(buffer);
			if (first_result.is_just())
				return first_result;

			auto second_result = m_second->parse(buffer);
			if (second_result.is_just())
				return second_result;

			return maybe<R>::nothing;
		}

	private:
		element_pointer m_first;
		element_pointer m_second;
	};

	//! Use two parsers immediately after one another.
	/*! Fails if either parser fails. Only returns the result of the second. */
	template<typename R, typename T, typename M>
	class sequence_combinator : public composite_combinator<R, T, R, M>
	{
	public:
		typedef typename composite_combinator<R, T, R, M>::major_pointer major_pointer;
		typedef typename composite_combinator<R, T, R, M>::minor_pointer minor_pointer;

	public:
		sequence_combinator(minor_pointer f, major_pointer s)
		: composite_combinator<R, T, R, M>(), m_first(f), m_second(s) {}

		sequence_combinator(const sequence_combinator&) = delete;
		~sequence_combinator() = default;

		maybe<R> parse(buffer<T>& buffer) const
		{
			auto start = buffer.here();

			auto first_result = m_first->parse(buffer);
			if (first_result.is_nothing())
				return maybe<R>::nothing;

			auto second_result = m_second->parse(buffer);
			if (second_result.is_just())
				return second_result;

			buffer.rewind(start);
			return maybe<R>::nothing;
		}

	private:
		minor_pointer m_first;
		major_pointer m_second;
	};

	//! Merge the output of two parsers.
	/*! Behaves like the sequence_combinator, but accumulates the results.
	 *  FIXME: This could be a composite combinator, might make it more useful.
	 */
	template<typename R, typename T>
	class merge_combinator : public uniform_combinator<typename accumulator<R>::result_type, T, R>
	{
	public:
		typedef typename accumulator<R>::result_type result_type;
		typedef typename uniform_combinator<result_type, T, R>::element_pointer element_pointer;

	public:
		merge_combinator(element_pointer f, element_pointer s)
		: uniform_combinator<result_type, T, R>(), m_first(f), m_second(s) {}

		merge_combinator(const merge_combinator&) = delete;
		~merge_combinator() = default;

		maybe<result_type> parse(buffer<T>& buffer) const
		{
			auto start = buffer.here();

			auto first_result = m_first->parse(buffer);
			if (first_result.is_nothing())
				return maybe<result_type>::nothing;

			accumulator<R> accum;
			accum.append(first_result.from_just());

			auto second_result = m_second->parse(buffer);
			if (second_result.is_just())
			{
				accum.append(second_result.from_just());

				auto total = accum.result();
				return maybe<result_type>::just(total);
			}

			buffer.rewind(start);
			return maybe<result_type>::nothing;
		}

	private:
		element_pointer m_first;
		element_pointer m_second;
	};

	//! Perform the action of a parser many times, combining the results.
	/*! Stops parsing and returns the result so far when the "max" limit is hit,
	 *  or when the parser fails. Fails if the "min" limit is not met.
	 */
	template<typename R, typename T>
	class many_combinator : public uniform_combinator<typename accumulator<R>::result_type, T, R>
	{
	public:
		typedef typename accumulator<R>::result_type result_type;
		typedef typename uniform_combinator<result_type, T, R>::element_pointer element_pointer;

	public:
		many_combinator(element_pointer p, std::size_t min, std::size_t max)
		: uniform_combinator<result_type, T, R>(), m_parser(p), m_min(min), m_max(max) {}

		many_combinator(const many_combinator&) = delete;
		~many_combinator() = default;

		maybe<result_type> parse(buffer<T>& buffer) const
		{
			auto start = buffer.here();

			std::size_t i = 0;
			accumulator<R> accum;

			//! A max of "0" means the max is unbounded.
			while (!m_max || i < m_max)
			{
				maybe<R> next = m_parser->parse(buffer);
				if (next.is_nothing())
					break;

				i += 1;
				accum.append(next.from_just());
			}

			if (i < m_min)
			{
				buffer.rewind(start);
				return maybe<result_type>::nothing;
			}

			auto repeated = accum.result();
			return maybe<result_type>::just(repeated);
		}

	private:
		element_pointer m_parser;
		std::size_t m_min, m_max;
	};

	//! Perform a group of parser actions in order.
	/*! Some parser actions may be bound to a name, and can be accessed
	 *  in a function called after parsing is complete.
	 */
	template<typename R, typename T, typename M>
	class block_combinator : public uniform_combinator<R, T, M>
	{
	public:
		typedef typename uniform_combinator<R, T, M>::element_pointer element_pointer;

	public:
		block_combinator()
		: m_statements() {}

		block_combinator(const block_combinator&) = delete;
		~block_combinator() = default;

		block_combinator& then(element_pointer p)
		{
			m_statements.push_back(p);
			return *this;
		}

		template<typename F>
		void evaluate(const F& f) { m_function = f; }

		maybe<R> parse(buffer<T>& buffer) const
		{
			auto start = buffer.here();

			std::map<std::string, M> bound;
			for (auto& p : m_statements)
			{
				auto result = p->parse(buffer);
				if (result.is_nothing())
				{
					buffer.rewind(start);
					return maybe<R>::nothing;
				}

				//! Only parsers with a valid tag have their results stored.
				if (p->tag().length())
					bound[p->tag()] = result.from_just();
			}

			auto final = m_function(bound);
			return maybe<R>::just(final);
		}

	private:
		std::vector<element_pointer> m_statements;
		//! The function is passed a const reference to a string map of results.
		std::function<R(const std::map<std::string, M>&)> m_function;
	};
}
}
