#pragma once
#include "parser.h"
#include "detail/combinator.h"
#include "detail/parser_traits.h"
namespace cpparse{
    using detail::parser_traits;
    template<typename R,typename T>
    using choice_combinator =
        typename parser_traits<detail::choice_combinator<R,T>>::parser_p;
    template<typename R,typename M,typename T>
    using sequence_combinator =
        typename parser_traits<detail::sequence_combinator<R,M,T>>::parser_p;
    template<typename R, typename T>
    using merge_combinator
    =typename parser_traits<detail::merge_combinator<R,T>>::parser_p;
    template<typename R,typename T>
    using many_combinator
    = typename parser_traits<detail::many_combinator<R,T>>::parser_p;
    template<typename R,typename M,typename T>
    using block_combinator
    =typename parser_traits<detail::block_combinator<R,M,T>>::parser_p;

    template<typename P> many_combinator<out_type<P>, in_type<P> >
        many(P p, const std::size_t& min=0,const std::size_t& max=0){
        return make_parser<many_combinator<out_type<P>,
                                           in_type<P>>>(p,min,max);
    }
    template<typename P> many_combinator<out_type<P>,in_type<P>>
        many1(P p, const std::size_t& max=0){
        return many<P>(p,1,max);
    }
    // block combinator perform a series of parser actions.
    ///////////////////////////////////////////////////////
    template<typename R,typename M,typename T>
    block_combinator<R,M,T> block(){
        return make_parser<block_combinator<R,M,T>>();
    }
    template<typename P>
    using out_set_type = typename std::vector<out_type<P> >;
    template<typename P>
    lift_parser<out_set_type<P>,out_type<P>,in_type<P> > lift_vector(P p){
        return
            lift<out_set_type<P>>(p,
                                  [](const out_type<P>& r){
                                      out_set_type<P> v;
                                      v.push_back(r);
                                      return v;
                                  });
    }
    template<typename P, typename S>
    block_combinator<out_set_type<P>, out_set_type<P>,in_type<P> >
    sep_by(P p, S s){
        std::cout<<"start sep by"<< std::endl;
        return block<out_set_type<P>, out_set_type<P>, in_type<P> >()
            ->* (lift_vector(p) << tag("first"))
            ->* ( many(s>>p) << tag("rest"))
            ^ [](const std::map<std::string, out_set_type<P> >& m){
            out_set_type<P> result;
            result.push_back(m.at("first")[0]);
            for(auto& e : m.at("rest"))
                result.push_back(e);
            return result;
        };
    }

    template<typename P,typename S>
    block_combinator<out_set_type<P> , out_set_type<P>,in_type<P>>
        end_by(P p, S s){
        return block<out_set_type<P>, out_set_type<P>, in_type<P>>()
            ->* (lift_vector(p) << tag("first") )
            ->* (many( s>> p) << tag("rest"))
            ->* (s)
            ^ [](const std::map<std::string,out_set_type<P>> & m){
            out_set_type<P> result;
            result.push_back(m.at("first")[0]);
            for(auto& e : m.at("rest"))
                result.push_back(e);
            return result;
        };     
    }

    template<typename B,typename P>
    struct block_converter{
        using element_p
        =typename B::element_type::element_p;
        static  element_p convert(P p){
            return covert_impl(p,std::is_same<out_type<P>,
                               out_type<element_p> >());
        }
        static element_p covert_impl(P p, std::true_type){return p;}
        static element_p covert_impl(P p, std::false_type){
            auto _type = [](const out_type<P>& v){
                return out_type<element_p>();
            };
            return lift<out_type<element_p> >(p,_type);;
        }
    };
    ////////////////////////////////////////////////////////////////
    /// operation
    ////////////////////////////////////////////////////////////////
    
    template<typename B, typename P>
    B operator->*(B b,P p){
        std::cout<<"start operator->*"<<std::endl;
        b->then(block_converter<B,P>::convert(p));
        std::cout<<"end operator->*"<<std::endl;
        return b;
    }
    template<typename B,typename F>
    B operator^(B b,const F& f){
        std::cout<<"start operator^"<<std::endl;
        b->evaluate(f);
        std::cout<<"end operator^"<<std::endl;
        return b;
    };
    
    // operation | 
    template<typename P>
    choice_combinator<out_type<P>,in_type<P> > operator|(
        P a,parser<out_type<P>, in_type<P> > b){
        return make_parser<
            choice_combinator<
                out_type<P>, in_type<P> > >(a,b);
    }
    // operation >>=
    template<typename P>
    merge_combinator<out_type<P>, in_type<P> >
        operator>>=(P a, parser<out_type<P>, in_type<P> > b){
        return make_parser<merge_combinator<out_type<P>, in_type<P> > >(a,b);
    }
    // operation >> 
    template<typename P, typename Q>
    sequence_combinator<out_type<Q>,out_type<P>,in_type<P> >
    operator>>(P a, Q b){
        std::cout<<"start operator>>"<<std::endl;
        return make_parser<
            sequence_combinator<out_type<Q>,out_type<P>,in_type<P> >>(a,b);
    }
    //operator *
    template<typename P> many_combinator<out_type<P>, in_type<P> >
    operator++(P p){
        return make_parser<many_combinator<out_type<P>,
                                           in_type<P> > >
            (p,0,0);
        //return many<P>(p);
    }
    // operator +
    template<typename P> many_combinator<out_type<P> , in_type<P> >
    operator+(P p){return many1(p);}
    // operator %
    template<typename P, typename S>
    block_combinator<out_set_type<P>, out_set_type<P>, in_type<P> >
    operator%(P p, S s){ return sep_by(p,s);}
    // operator <<
    template<typename P>
    P operator<<(const P& p, const parser_tag& t){
        auto tagged = make_parser<P>(*p);
        tagged->set_tag(t.string);
        return tagged;
    }
    
    
}


