#pragma once
#include "detail.h"
namespace cpparse{
    template<typename P>
    lift_parser<std::vector<out_type<P> >,
                in_type<P>,
                out_type<P> > lift_vector(P p){
        return lift<std::vector<out_type<P> > >(
            p,
            [](const out_type<P>& r){
                std::vector<out_type<P>> v;
                v.push_back(r);
                return v;
            });
    }
    template<typename P, typename S>
    block_combinator<std::vector<out_type<P> >,
                     in_type<P>,
                     std::vector<out_type<P> > > sep_by(P p, S s){
        return block<std::vector<out_type<P > >,
                     in_type<P>,
                     std::vector<out_type<P> > >()
            -> *(lift_vector(p) << tag("first"))
            -> *(many(s >>p) << tag("rest"))
            ^ [](const std::map<std::string,
                 std::vector<out_type<P > > > & m){
            std::vector<out_type<P> >  result;
            result.push_back(m.at(first)[0]);
            for(auto& e: m.at("rest"))
                result.push_back(e);
            return result;
        }
    }
    
}
