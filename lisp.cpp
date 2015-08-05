//#include "buffer.h"
//#include "detail.h"
#include "parser.h"
#include "combinator.h"
#include "string_parser.h"
#include<readline/readline.h>
#include<readline/history.h>
#include<iostream>
#include<memory>
#include<numeric>
#include<sstream>
#include<vector>
struct lenv;
struct lval{
  lval* eval(){return this;}
  lval(const lval& )=default;
  virtual lval* eval(lenv& l){ return this;}
  virtual lval* reduce(std::vector<lval*>& l){return this;}
  virtual const std::string str(){return "";}
  virtual bool is_atom(){return false;}
  lval()=default;
};
using vals= std::vector<lval> ;

struct lenv:public std::map<std::string,lval*>{
};
using  children_type = std::vector<lval* >;
using  return_value = lval*;
// A symbol is interpreted as a variable name; its value is the
// variable's value

struct lval_sym: public lval{
  std::string  _sym;
  lval_sym(const std::string& sym):lval(),_sym(sym){}
  lval* eval(lenv& env){return env[_sym];}
  const std::string& str()const { return _sym;}
};
// A number evaluate to itself
struct lval_num :public lval{
  long int _num;
  lval_num(const long& num):lval(),_num(num){}
  const std::string str() const { return std::to_string(_num);}
};

struct lval_str: public lval{
  std::string  _str;
  lval_str(const std::string& sym):lval(),_str(sym){}
  const std::string str() const { return _str;}
};
//
struct lval_expr:public lval{
  children_type _children;
  lval_expr(const children_type& children):lval(),_children(children){}
  
  const std::string str() const {
    std::stringstream ss;
    ss<<"( ";
    for(int i=0;i<_children.size();i++){
      ss<<_children[i]->str()<<" ";
    }
    ss<<" )";
    return ss.str();
  }
  void _expr_print(const children_type& l,char l1,char l2 ){}
};
//conditional
struct lval_cond:public lval_expr{
  lval_cond(lval_expr* l):lval_expr(l->_children){}
  lval* eval(lenv& e){
    bool  l= _children[1]->eval(e);
    if(l){
      _children[2]->eval(e);
    }else{
      _children[3]->eval(e);
    }
  }
};
//define a new variable and give it the value of evaluating the
//expression
struct lval_def:public lval_expr{
  lval_def(lval_expr* l):lval_expr(l->_children){}
  lval* eval(lenv& env){
    std::string str = _children[1]->str();
    env[str] = _children[2]->eval(env);
    return env[str];
  }
};

struct lval_sexpr:public lval_expr{
  lval_sexpr(const children_type& children):lval_expr(children){}
  lval* eval(lenv& e){
    lval* proc = _children[0]->eval(e);
    delete _children[0];
    std::vector<lval*> vec(_children.size()-1);
    for(int i=1;i<_children.size();++i){
      vec[i-1] = _children[i]->eval(e);
      delete _children[i];
    }
    return proc->reduce(vec);
  }
};

struct lval_qexpr:public lval_expr{
  lval_qexpr(const children_type& children):lval_expr(children){}
  lval* eval(lenv& e){
    lval* l = new lval_sexpr(_children);
    return l;
  }
};

lval length(const std::vector<lval>& l){ return lval_num(l.size());}
lval rest(const std::vector<lval>& l){
  
  return lval_expr(l.begin()+1,l.end());
}

std::shared_ptr<lval> lx_to_int(const std::string& content){
  return std::make_shared<lval_num>(std::stoi(content));
}
lval lx_to_sym(const std::string& content){
  return  lval_sym(content);
}
return_value lx_to_string(const std::string& content){
  return new lval_str(content);
}
return_value lx_to_sexpr(const std::vector<return_value>& ves){
  return new lval_sexpr(ves);
}
return_value lx_to_qexpr(const std::vector<return_value>& ves){
  return new lval_qexpr(ves);
}
int main(){
    
  using namespace cpparse;
  using lisp_parser
      = parser<return_value,std::string>;
  //atom parser
  lisp_parser number = lift<return_value>(
      +digit(),lx_to_int);
  lisp_parser sym = lift<return_value>(
      lift_string(letter() |symbol())
      >>= ++(letter()|digit()|symbol()), lx_to_sym);
  lisp_parser string_ = lift<return_value>(
      lx_string(), lx_to_string);
  ///////////////////
  auto expr = placeholder<return_value,std::string>();
  auto sexpr =inside(character('('),
                     lift<return_value>(expr % spaces(), lx_to_sexpr)
                     ,character(')'));
  auto qsexpr = inside(string("'("),
                       lift<return_value>(expr % spaces(), lx_to_qexpr),
                       character(')'));
  //( )
  expr &= (sym|number)|string_|sexpr|qsexpr;
  puts("Lisp Version ");
  puts("Press Ctrl+c to Exit\n");
  while(true){
    char* input = readline("lisp> ");
    add_history(input);
    buffer<std::string> buffer(input);
    auto res = expr->parse(buffer);
    if(res){
      std::cout<<(*res)->str()<<std::endl;
    }else{
      std::cout<<"I can't expans";
      puts(input);
    }
    free(input);
  }
  return 0;
}
