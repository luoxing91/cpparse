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
enum cell_type{Symbol, Number, List, Proc, Lambda};
struct environment;
struct cell{
  using proc_type = std::function<cell(const std::vector<cell>&);
  using iter = std::vector<cell>::const_iterator iter;
  using map = std::map<std::string, cell>;
};
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
