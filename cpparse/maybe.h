#pragma once

#include <stdexcept>

namespace cpparse
{
	//! Store an optional value.
	template<typename T>
	class maybe
	{
	public:
		typedef T value_type;

	public:
		//! Should always be constructed with these static functions.
		static maybe just(const value_type&);
		static const maybe nothing;

	public:
		maybe() : m_value(nullptr) {}
		maybe(const value_type& v) 
		: m_value(new (m_storage) value_type(v)) {}
		
		maybe(const maybe& other) { *this = other; }
		~maybe() = default;

		maybe& operator=(const maybe& other)
		{
			if (other.m_value)
				m_value = new (m_storage) value_type(other.from_just());
			else
				m_value = nullptr;

			return *this;
		}

		const value_type& from_just() const
		{
			if (!m_value)
				throw std::runtime_error("cpparse::maybe : Cannot unwrap maybe::nothing!");

			return *m_value;
		}

		value_type& from_just()
		{
			auto const_this = static_cast<const maybe*>(this);
			return const_cast<value_type&>(const_this->from_just());
		}

		bool is_just() const { return (m_value); }
		bool is_nothing() const { return (!m_value); }

		const value_type& operator*() const { return from_just(); }
		value_type& operator*() { return from_just(); }

		//! Implicit boolean conversion.
		operator bool() { return is_just(); }

	private:
		//! Use a char array so an empty T is never intialized.
		char m_storage[sizeof(value_type)];
		value_type* m_value;
	};

	template<typename T>
	maybe<T> maybe<T>::just(const T& value) { return maybe<T>(value); }
	template<typename T>
	const maybe<T> maybe<T>::nothing = maybe<T>();
}
