//#include "buffer.h"
//#include "detail.h"
#include "parser.h"
#include "combinator.h"
#include "string_parser.h"
#include<readline/readline.h>
#include<readline/history.h>
#include<iostream>
#include<memory>
struct lisp_token{
    virtual void show()=0;
    virtual int result(){return 0;}
};
using token_pointer = std::shared_ptr<lisp_token> ;
template<typename T>
struct lisp_atom :public lisp_token{
    T name;
    lisp_atom(const T& n):name(n){}
    const T& result() const {return name;}
    void show(){std::cout<<"lisp atom: "<<name<<std::endl;}
};
using lisp_bool = lisp_atom<bool>;
using lisp_number = lisp_atom<int>;
using lisp_string = lisp_atom<std::string>;
struct lisp_symbol:public lisp_token{
    lisp_symbol(const std::string& name):_name(name){}
    std::string _name;
    int result(){return 0;}
    void show(){std::cout<<"lisp_symbol: "<<_name<<std::endl;}
};
struct lisp_cons:public lisp_token{
    lisp_cons(token_pointer head,token_pointer tail):
        _head(head),_tail(tail){}
    token_pointer _head,_tail;
    void show(){_head->show();}
};
struct lisp_list: public lisp_token{
    std::vector<token_pointer> items;
    lisp_list(const std::vector<token_pointer>& i):items(i){}
    void show(){
        std::cout<<result()<<std::endl;
        for(auto& x: items){
            x->show();
        }
    }
    
    
};
struct lisp_dotted:public lisp_token{
    std::vector<token_pointer> items;
    token_pointer tail;
    lisp_dotted(const std::vector<token_pointer> i):items(i){}
    void show(){
        for(auto& x: items){
            x->show();
        }
    }
};
template<typename T>
token_pointer make_token(const T& val){
    return std::make_shared<lisp_atom<T>>(val);
}
token_pointer make_symbol(const std::string& name){
    return std::make_shared<lisp_symbol>(name);
}
struct lisp_nil:public lisp_token{
    static token_pointer nil;
};

token_pointer lx_to_int(const std::string& s){
    int value = atoi(s.c_str());
    return make_token(value);
}
token_pointer lx_to_list(const std::vector<token_pointer>& v){
    return token_pointer(new lisp_list(v));
}
token_pointer lx_to_atom(const std::string& s){
    if(s=="#t") return make_token(true);
    else if(s=="#f") return make_token(false);
    return make_symbol(s);
}
token_pointer lx_to_string(const std::string& str){
    return token_pointer(new lisp_string(str));
}

int main(){
    using namespace cpparse;
    using lisp_parser
        = parser<token_pointer,std::string>;
    //atom parser
    lisp_parser number = lift<token_pointer>(
            +digit(),lx_to_int);
    lisp_parser atom = lift<token_pointer>(
        lift_string(letter() |symbol())
        >>= ++(letter()|digit()|symbol()), lx_to_atom);
    lisp_parser string_ = lift<token_pointer>(
        lx_string(), lx_to_string);
    ///////////////////
    auto expr = placeholder<token_pointer,std::string>();
    auto list = lift<token_pointer>(
        (expr % spaces()), lx_to_list );
    auto paren = inside(character('('),list,character(')'));
    //recurse->set_target(expr);
    expr &= (atom|number)|string_|paren;
    puts("Lisp Version ");
    puts("Press Ctrl+c to Exit\n");
    while(true){
        char* input = readline("lisp> ");
        add_history(input);
        buffer<std::string> buffer(input);
        auto res = expr->parse(buffer);
        if(res){
            (*res)->show();
        }else{
            std::cout<<"I can't expans";
            puts(input);
        }
        free(input);
    }
    return 0;
}
