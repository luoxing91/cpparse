#include <iostream>

#include "../cpparse/cpparse.h"

using namespace cpparse;

struct lisp_token
{
	virtual void show() = 0;
};

typedef std::shared_ptr<lisp_token> token_pointer;

struct lisp_atom : public lisp_token
{
	std::string name;

	lisp_atom(const std::string& n) : name(n) {}
	void show() { std::cout << "atom: " << name << std::endl; }
};

struct lisp_bool : public lisp_token
{
	bool value;

	lisp_bool(bool v) : value(v) {}
	void show() { std::cout << "bool: " << value << std::endl; }
};

struct lisp_number : public lisp_token
{
	int value;

	lisp_number(int v) : value(v) {}
	void show() { std::cout << "number: " << value << std::endl; }
};

struct lisp_string : public lisp_token
{
	std::string value;

	lisp_string(const std::string& v) : value(v) {}
	void show() { std::cout << "string: \'" << value << "\'" << std::endl; }
};

struct lisp_list : public lisp_token
{
	std::vector<token_pointer> items;

	lisp_list(const std::vector<token_pointer> i) : items(i) {}
	void show()
	{
		std::cout << "vvvvv" << std::endl;
		for (auto& t : items)
			t->show();
		std::cout << "^^^^^" << std::endl;
	}
};

struct lisp_dotted : public lisp_token
{
	std::vector<token_pointer> items;
	token_pointer tail;

	lisp_dotted(const std::vector<token_pointer> i) : items(i) {}
	void show()
	{
		std::cout << "vvvvv" << std::endl;
		for (auto& t : items)
			t->show();
		std::cout << "^^^^^" << std::endl;

		tail->show();
		std::cout << "-----" << std::endl;
	}
};

// compile and run: g++ -std=c++11 -o lisp lisp.cpp && ./lisp
int main()
{
	auto recurse = placeholder<token_pointer, std::string>();

	auto atom_str = lift_string(letter() | symbol()) >>= many(letter() | digit() | symbol());
	auto atom_lift = lift<token_pointer>(atom_str,
		[](const std::string& s)
		{
			if (s == "#t") return token_pointer(new lisp_bool(true));
			if (s == "#f") return token_pointer(new lisp_bool(false));
			return token_pointer(new lisp_atom(s));
		});

	auto number_str = many1(digit());
	auto number_lift = lift<token_pointer>(number_str,
		[](const std::string& s)
		{
			int value = atoi(s.c_str());
			return token_pointer(new lisp_number(value));
		});

	auto string_lift = block<token_pointer, std::string, std::string>()
		->* ( character('\"')                      )
		->* ( many(none_of("\"")) << tag("inside") )
		->* ( character('\"')                      )
		^ [](const std::map<std::string, std::string>& m)
		{
			auto str = m.at("inside");
			return token_pointer(new lisp_string(str));
		};

	auto list_vec = sep_by(recurse, spaces());
	auto list_lift = lift<token_pointer>(list_vec,
		[](const std::vector<token_pointer>& v)
		{
			return token_pointer(new lisp_list(v));
		});

	auto dotted_lift = block<token_pointer, std::string, std::vector<token_pointer>>()
		->* ( end_by(recurse, spaces())  << tag("head") )
		->* ( character('.') >> spaces()                )
		->* ( lift_vector(recurse)       << tag("tail") )
		^ [](const std::map<std::string, std::vector<token_pointer>>& m)
		{
			auto dotted = new lisp_dotted(m.at("head"));
			dotted->tail = m.at("tail")[0];

			return token_pointer(dotted);
		};

	auto paren_parse = block<token_pointer, std::string, token_pointer>()
		->* ( character('(')                            )
		->* ( (dotted_lift | list_lift) << tag("inner") )
		->* ( character(')')                            )
		^ [](const std::map<std::string, token_pointer>& m)
		{
			return m.at("inner");
		};

	auto expr = atom_lift | number_lift | string_lift | paren_parse;
	recurse->set_target(expr);

	buffer<std::string> buf("(+ (- 4 2) 3 1 . #t)");
	auto res = expr->parse(buf);

	if (res.is_just())
		(*res)->show();

	return 0;
}
