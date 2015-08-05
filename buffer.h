#pragma once
#include <boost/optional.hpp>
namespace cpparse{

    using boost::optional;
    template<typename T>
    struct buffer{
        typedef T container_t;
        typedef typename container_t::const_iterator iterator;
        typedef typename iterator::value_type value_t;
        container_t _data;
        iterator _current;
        buffer(const container_t& d):_data(d),_current(d.begin()){}
        bool has_next() const {return _current != _data.end();}
        optional<value_t> next(){
            if(!has_next()) return optional<value_t>();
            value_t t = *_current;
            _current++;
            return optional<value_t>(t);
        }
        iterator here() const { return _current;}
        void rewind(const iterator& to){_current =to;}
        value_t operator*() const {return *_current;}
    };
}
