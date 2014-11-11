#pragma once

#include <iterator>

#include "maybe.h"

namespace cpparse
{
	//! Manage position along an iterator for the given type.
	template<typename T>
	class buffer
	{
	public:
		typedef T container_type;
		typedef typename container_type::const_iterator iterator;
		typedef typename iterator::value_type value_type;

	public:
		buffer(const container_type& d)
		: m_data(d), m_current(m_data.begin()) {}
		
		buffer(const buffer&) = default;
		~buffer() = default;

		bool has_next() const { return (m_current != m_data.end()); }
		maybe<value_type> next()
		{
			if (!has_next())
				return maybe<value_type>::nothing;

			return maybe<value_type>::just(*(m_current++));
		}

		iterator here() const { return m_current; }
		void rewind(const iterator& to) { m_current = to; }

		value_type operator*() const { return *m_current; }

	private:
		container_type m_data;
		iterator m_current;
	};
}
