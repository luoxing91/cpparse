#pragma once

#include "detail/join.h"

namespace cpparse
{
	//! Combine output over multiple instances.
	template<typename R>
	class accumulator
	{
	public:
		typedef typename detail::join<R>::result_type result_type;

	public:
		accumulator()
		: m_result(result_type()) {}

		accumulator(const accumulator&) = default;
		~accumulator() = default;

		void append(const R& v) { detail::join<R>::append(m_result, v); }

		const result_type& result() const { return m_result; }
		result_type result() { return m_result; }

		void operator+=(const R& v) { append(v); }

		const result_type& operator*() const { return result(); }
		result_type operator*() { return result(); }

	protected:
		result_type m_result;
	};
}
