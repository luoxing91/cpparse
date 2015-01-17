#pragma once

#include <vector>

#include "parser.h"
#include "combinator.h"

namespace cpparse
{
	//! Convert the output of any parser from R to vector<R>.
	template<class P>
	lift_parser<std::vector<out_type<P>>, in_type<P>, out_type<P>> lift_vector(P p)
	{
		return lift<std::vector<out_type<P>>>(p,
			[](const out_type<P>& r)
			{
				std::vector<out_type<P>> v;
				v.push_back(r);

				return v;
			});
	}

	//! Parse a sequence of P parsers, whose input is separated by a parser S.
	template<class P, class S>
	block_combinator<std::vector<out_type<P>>, in_type<P>, std::vector<out_type<P>>> sep_by(P p, S s)
	{
		return block<std::vector<out_type<P>>, in_type<P>, std::vector<out_type<P>>>()
			->* ( lift_vector(p) << tag("first") )
			->* ( many(s >> p)   << tag("rest")  )
			^ [](const std::map<std::string, std::vector<out_type<P>>>& m)
			{
				std::vector<out_type<P>> result;
				result.push_back(m.at("first")[0]);
				for (auto& e : m.at("rest"))
					result.push_back(e);

				return result;
			};
	}

	//! Same as the sep_by combinator, but the separator S must be present after the last P.
	template<class P, class S>
	block_combinator<std::vector<out_type<P>>, in_type<P>, std::vector<out_type<P>>> end_by(P p, S s)
	{
		return block<std::vector<out_type<P>>, in_type<P>, std::vector<out_type<P>>>()
			->* ( lift_vector(p) << tag("first") )
			->* ( many(s >> p)   << tag("rest")  )
			->* ( s                              )
			^ [](const std::map<std::string, std::vector<out_type<P>>>& m)
			{
				std::vector<out_type<P>> result;
				result.push_back(m.at("first")[0]);
				for (auto& e : m.at("rest"))
					result.push_back(e);

				return result;
			};
	}
}
