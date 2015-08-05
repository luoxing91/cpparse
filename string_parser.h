#pragma once
#include "parser.h"
#include "detail/parser_traits.h"
#include "detail/string_parser.h"
#include "combinator.h"

//#include ""
namespace cpparse{
    using char_parser = parser_traits<detail::char_parser>::parser_p;
    char_parser character(char c){
        return make_parser<char_parser>(c);
    }
    noneof_char_parser not_character(char c){
        return none_of(std::string(1,c));
    }
    using oneof_char_parser = oneof_parser<char,std::string>;
    oneof_char_parser one_of(const std::string& s){
        return one_of<std::string>(std::vector<char>(s.begin(),s.end()));
    }
    using string_parser =
        typename parser_traits<detail::string_parser>::parser_p;
                               
    string_parser string(const std::string& s){
        return make_parser<string_parser>(s);
    }
    /////////////////////////
    oneof_char_parser upper(){ return one_of("ABCDEFGHIGKLMNOPQRSTUVWXYZ");}
    oneof_char_parser lower(){ return one_of("abcdefghijklmnopqrstuvwxyz");}
    oneof_char_parser digit(){ return one_of("1234567890");}
    oneof_char_parser symbol(){return one_of("!#$%&|*+-/:<=>?@^_~");}
    
    //////////////////////////////////////////////////////////////////////
    lift_parser<std::string,char,std::string>
    lift_string(parser<char,std::string> p){
        return lift<std::string>
            (p,[](const char& c){
                return std::string(1,c);
            });
    }
    
    choice_combinator<char,std::string> letter(){return upper()|lower();}
    ///////////////////////////////////
    using many_char_combinator = many_combinator<char,std::string>;
    many_char_combinator many(parser<char,std::string> p, std::size_t min=0,
                              std::size_t max=0){return many(p,min,max);}
    many_char_combinator many1(parser<char,std::string>p, std::size_t max=0){
        return many1(p,max);
    }
    many_char_combinator spaces(){return many1(one_of(" \t\n\r"));}

    
    using merge_string_combinator
    = merge_combinator<std::string,std::string>;
    
    merge_string_combinator operator>>=(parser<std::string,std::string> a,
                                        parser<std::string,std::string> b){
        return make_parser<merge_string_combinator>
            (a,b);
    }
    ///////////////////////////////////////////////////
    parser<int,std::string> lx_int(){
        return lift<int>(many1(digit()),
                         [](const std::string& s){
                             return atoi(s.c_str());
                         });
    }
    ///////////////////////////////////////////////////////
    block_combinator<std::string,std::string,std::string> lx_string(){
        return block<std::string, std::string,std::string>()
            ->* (character('\"'))
            ->* (++none_of("\"") << tag("inside"))
            ->* (character('\"'))
            ^ [](const std::map<std::string,std::string>& m){
            return m.at("inside");
        };
    }
    //in_type Q1,,in_type Q2 must be same
    template<typename P, typename Q1,typename Q2>
    block_combinator<out_type<P>,out_type<P>,in_type<Q1>>
        inside(Q1 q1,P p, Q2 q2){
        return block<out_type<P>, out_type<P>,in_type<Q1> >()
            ->*q1
            ->* (p<< tag("inside"))
            ->* q2
            ^ [](const std::map<std::string,out_type<P> >& m){
            return m.at("inside");
        };
    }
}
