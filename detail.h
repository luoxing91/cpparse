#pragma once
#include<algorithm>
#include<vector>
#include<string>
#include<functional>
#include<map>
#include<memory>
namespace cpparse{
        //dtodo
        ///////////////////////////////
    template<typename R>
    struct join{
        typedef std::vector<R> result_t;
        static void append(result_t& res, const R& v){ res.push_back(v);}
    };
    template<>
    struct join<char>{
        typedef std::string result_t;
        static void append(result_t& res, const char& v){ res +=v;}
    };
    template<>
    struct join<std::string>{
        typedef std::string result_t;
        static void append(result_t& res, const std::string& v){ res+=v;}
    };
    template<typename R>
    struct accumulator{
        typedef typename join<R>::result_t result_t;
        accumulator():_r(result_t()){}
        void append(const R& v){join<R>::append(_r,v);}
        const result_t& result() const {return _r;}
        result_t _r;
    };
    //generic parser interface takes an input type and output
      

}
        

