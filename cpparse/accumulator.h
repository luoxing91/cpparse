#pragma once

#include <vector>
#include <string>

namespace cpparse
{
	//! Combine output over time.
	/*! All subclasses of accumulator should use have only one template
	 *  parameter; the R type should be constant based on T and the
	 *  implementation of the subclass.
	 */
	template<typename R, typename T>
	class accumulator
	{
	public:
		typedef T value_type;
		typedef R result_type;

	public:
		accumulator()
		: m_result(R()) {}

		accumulator(const accumulator&) = default;
		virtual ~accumulator() = default;

		virtual void append(const T&) = 0;

		const result_type& result() const { return m_result; }
		result_type result()
		{
			auto const_this = static_cast<const accumulator*>(this);
			return const_cast<result_type&>(const_this->result());
		}

		void operator+=(const value_type& v) { append(v); }

		result_type operator*() { return result(); }
		const result_type& operator*() const { return result(); }

	protected:
		result_type m_result;
	};

	//! Combine output using the += operator.
	template<typename T>
	class concat_accumulator : public accumulator<T, T>
	{
	public:
		concat_accumulator()
		: accumulator<T, T>() {}

		concat_accumulator(const concat_accumulator&) = default;
		~concat_accumulator() = default;

		void append(const T& v)
		{
			this->m_result += v;
		}
	};

	//! Insert each append into a vector.
	template<typename T>
	class vector_accumulator : public accumulator<std::vector<T>, T>
	{
	public:
		vector_accumulator()
		: accumulator<std::vector<T>, T>() {}

		vector_accumulator(const vector_accumulator&) = default;
		~vector_accumulator() = default;

		void append(const T& v)
		{
			this->m_result.push_back(v);
		}
	};

	//! Combine all input into a string.
	/*! Similar to the concat accumulator, but instead of combining
	 *  T and T, any type that can be combined into a string can be used.
	 */
	template<typename T>
	class string_accumulator : public accumulator<std::string, T>
	{
	public:
		string_accumulator()
		: accumulator<std::string, T>() {}

		string_accumulator(const string_accumulator&) = default;
		~string_accumulator() = default;

		void append(const T& v)
		{
			this->m_result += v;
		}
	};
}
