
#pragma once

#include "buffer.h"
#include "core.h"
#include "detail.h"
#include "detail/parser.h"
#include "detail/combinator.h"
#include<memory>
#include<utility>
namespace cpparse{
    using detail::parser_traits;
    
    template<typename R, typename T>
    using parser = typename
        detail::parser_traits<detail::parser<R,T>>::parser_p;

    //some untities to make transferring type info easier.
    template<typename P>
    using in_type =
        typename detail::parser_traits<typename P::element_type >::value_t;
    template<typename P>
    using out_type = typename
        parser_traits<typename P::element_type >::result_t;

    template<typename P, typename...Args>
    P make_parser(Args&&...args){
        return std::make_shared<typename P::element_type>
            (std::forward<Args>(args)...);
    }
    struct parser_tag {std::string string;};
    parser_tag tag(const std::string& s){ return {s};}
    
    template<typename R, typename T>
    using forward_parser =
                typename parser_traits<detail::forward_parser<R, T>>::parser_p;
    //! The template arguments always have to be specified for this function.
    template<typename R, typename T>
    forward_parser<R, T> placeholder(){
        return make_parser<forward_parser<R, T>>();
    }
    template<typename P, typename S>
    P operator&=(P p , S t){
        p->set_target(t);
        return p;
    }
    template<typename T, typename M>
    using skip_parser =
                typename parser_traits<detail::skip_parser<T, M>>::parser_p;

    /* All template utility function have to define the input parser
     * type with a template class instead of typename R, typename T -
     * the compiler is otherwise unable to infer the type since it is
     * wrapped in a shared_ptr that cannot be implicitly converted
     * from a container for the dervied class to a container for the
     * base class.
     */
    template<class P>
    skip_parser<out_type<P>,in_type<P>> skip(P p){
        return make_parser<skip_parser<out_type<P>,in_type<P> >>(p);
    }
    // ******************************************************************
    //! Option Parser - provide alternative output upon parser failure.
    // ******************************************************************
    template<typename R, typename T>
    using option_parser =
                typename parser_traits<detail::option_parser<R, T>>::parser_p;

    template<class P>
    option_parser<out_type<P>, in_type<P>> option(P p, const out_type<P>& a){
        return make_parser<option_parser<out_type<P>, in_type<P>>>(p, a);
    }
    
    template<class P>
    option_parser<out_type<P>, in_type<P>> _optional(P p){
        return option(p, out_type<P>());
    }
    template<typename P>
    option_parser<out_type<P>, in_type<P>> operator!(P p){
        return _optional(p);
    }
    // ******************************************************************
    //! Lift Parser - map the result of a parser to a new type.
    // ******************************************************************
    template<typename R, typename M,typename T>
    using lift_parser
    = typename parser_traits<detail::lift_parser<R, M, T>>::parser_p;
                             

    //! The R parameter must always be specified, but the others will
    //! be deduced.
    template<typename R, typename F, class P>
    lift_parser<R, out_type<P> ,in_type<P>> lift(P p, const F& f){
        return make_parser<lift_parser<R, out_type<P>,in_type<P> >>(p, f);
    }
    
    // ******************************************************************
    //! OneOf Parser - match any one of a list of values to the input.
    // ******************************************************************
    template<typename R, typename T>
    using oneof_parser = typename
                parser_traits<detail::oneof_parser<R, T>>::parser_p;
    template<typename T, typename R>
    oneof_parser<R, T> one_of(const std::vector<R>& c){
        return make_parser<oneof_parser<R, T>>(c);
    }

    // ******************************************************************
    //! NoneOf Parser - match any token except those in a list of values.
    // ******************************************************************
    template<typename R, typename T>
    using noneof_parser =typename
                parser_traits<detail::noneof_parser<R, T>>::parser_p;

    template<typename T, typename R>
    noneof_parser<R, T> none_of(const std::vector<R>& r){
        return make_parser<noneof_parser<R, T>>(r);
    }
    using noneof_char_parser = noneof_parser<char,std::string>;
    noneof_char_parser none_of(const std::string& s){
        return make_parser<noneof_parser<char, std::string>>(s);
    }
    
                         

}
