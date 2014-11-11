CPPARSE
=
cpparse is a small combinator/parser utility using templates and C++11, based off of Haskell's [Parsec](https://www.haskell.org/haskellwiki/Parsec) library.

BASICS
-
cpparse operates around three main classes:

- `buffer<T>`: Maintains the current position along an input value, which is only advanced upon a successful parse.
  - `buffer::buffer(const T&)`: construct a buffer containing the given input.
- `maybe<T>`: The class's value can only be accessed when it is `just`. `nothing` means no value is contained.
  - `bool is_just()`/`bool is_nothing()`: test if a value is contained. Implicit conversion to `bool` is also possible.
  - `T from_just()`/`T operator*()`: retrieve the value contained. Throws an exception if the class is `nothing`.
- `parser<R, T>`: All parser classes extend from this interface, which takes an input type `T` and an output type `R`.
  - `maybe<T> parser::parse(buffer<T>&)`: apply the parser to the given input.

### Example

    using namespace cpparse;

    auto my_parser = string("hello"); //< Create a string parser.
    buffer<std::string> buf("hello world");

    // Apply the parser to the input string.
    auto result = my_parser->parse(buf);
    if (result)
        std::cout << *result << std::endl;

will output `hello`, and the buffer will contain, from its current position, `" world"`.

PARSERS
-
cpparse includes some default common parser behaviors.

### Char & String Parsers

The basic units for parsing string input. Created with the `string` and `character` functions.

    auto ch_parser = character('c');
    
Applying `ch_parser` to a buffer containing `"cello"` would return a character variable with value `'c'`. Similarly, a string parser returns an `std::string` value equal to the parser's input string.

### OneOf Parsers

The `one_of` function takes an `std::vector` of values and tries to match the input to a single one of those values. This function also has a character parser override that takes an `std::string`, and tries to match one of its characters with the input.

    auto some_ch = one_of("abcd");

Applying `some_ch` to a buffer of `"develop"` would return `'d'`, while applying it to `"apple"` would return `'a'`.

For the opposite behavior of the OneOf Parser, `none_of` behaves as described above, but matches any value not specified.

### Option Parsers

The `option` function takes another parser as input, along with an optional value. It behaves the same way as the parser it was passed, but upon failure, returns the optional value instead of `maybe::nothing`.

    auto perhaps = option(string("hello"), "see ya");

When given an input of `"hello world"`, the parser returns `"hello"`, but with input such as `"goodbye world"`, the parser will return `"see ya"` instead.

An additional function, `optional`, takes no alternate value, and returns an empty type upon failure (`""` for `std::string`).

### Skip Parsers

The `skip` function allows output to be ignored upon success.

    auto ignore = skip("misc");

With the input `"misc info"`, the parser simply returns an empty type value, in this case, `""`. This is mainly useful when working with other parsers in a combinator.

### Forward Parsers

The `placeholder<R, T>` function allows an empty parser to be defined and set later (with `set_target`). Note that template arguments must be explicitly specified.

    auto recurse = placeholder<std::string, std::string>();
    recurse->set_target(string("iterate"));

When `recurse` is used, the string parser will be applied to the input. This can be used for recursive parsing when used in conjunction with combinators.

### Lift Parsers

The `lift<R>` function takes an existing parser, and transforms its result into a new type/value with a lambda or function pointer. The new return type of the parser must be specified as a template argument, and the passed lambda's parameter type must be equal to the result type of the inner parser.

    auto to_int = lift<int>(one_of("1234567890"),
        [](const std::string& s)
        {
            int i = atoi(s.c_str());
            return i;
        });

When applied to the string `"4"`, the above parser will return the integer `4`.

COMBINATORS
-
cpparse allows for parsers to be combined to create more complex behaviors.

### Many Combinators

The `many` and `many1` functions allow a parser to be applied repeatedly to the input. `many` will still succeed if no input is matched, while `many1` requires the parser is applied once to succeed.

    auto sleeping = many(character('z'));

When applied to `"zzz"`, the string `"zzz"` is returned. Note that using `many` with a character parser automatically converts the result to a string. 

When `many` is applied to other input types, a template parameter `accumulator` must be specified. Available options are:

- `vector_accumulator`: returns an `std::vector` containing an entry for each parse result.
- `concat_accumulator`: tries to combine its output with the `+=` operator.

### Choice Combinators

The `|` operator attempts to apply parsers to the input, in order, until one succeeds, returning that value. If all parsers fail, `maybe::nothing` is returned.

All parsers in the choice must be the same type.

    auto greeting = string("hello") | string("hola");

The `greeting` parser will first attempt to match `"hello"` to the input, and then tries `"hola"` if that fails.

### Sequence Combinators

The `>>` operator applies multiple combinators in the order given. When all parsers are successful, the result of the last parser in the chain is returned. If one of the parsers fails, the chain stops executing, and `maybe::nothing` is returned. 

Note that each element of the sequence can be a different parser type.

    auto alphabet = character('a') >> string("b") >> character('c');

When the above parser is applied to the input `"abcd"`,  `'c'` is returned. However, if it is applied to `"adc"`, the parser fails and `maybe::nothing` is returned.

### Merge Combinators

The `>>=` operator behaves similarly to the sequence combinator, but combines the output of each parser into one result. This is where the `skip` function is most useful.

Each merged parser must be of the same type.

    auto squished = string("no") >>= skip(string(" ")) >>= string("space");

When applied to the string `"no space"`, the above parser would return the string `"nospace"`.

### Block Combinators

The `block()` combinator works similarly to the sequence combinator as well, but includes a special function at the end to transform the parser results.

This combinator ended up being rather complicated trying to imitate Haskell's "do" notation. Use of the block combinator requires three steps during its creation:

- Call the `block<R, T, M>()` function: This function requires three template parameters: the final output type, the input type, and the output type of the primary parser statements.
- Specify the steps with the `->*` operator: each parser will be executed in the order given.
- Give a processing function with the `^` operator: specify, with a lambda or function pointer, how the data from the parsers should be combined/used.

To carry data between the parser sequence and the processing function, all parsers can be "tagged" with a name (using the `<<` operator and the `tag()` function). When the block combinator is running, output from each parser with a valid tag is placed in an `std::map<std::string, M>`, mapping the tag name to the result. This map is then passed to the processing function.

For maximum flexibility, the block combinator accepts parsers of all return types. However, tagged values will only be stored  if they match the `M` type specified in the `block()` statement.

    auto html_name = block<std::string, std::string, std::string>()
        ->* ( character('<')                     )
        ->* ( many(none_of(">")) << tag("inner") )
        ->* ( character('>')                     )
        ^ [](const std::map<std::string, std::string>& m)
        {
            return m.at("inner");
        };

When presented with the input `"<table>"`, the string `"table"` will be returned.

UTILITY FUNCTIONS
-
cpparse contains some pre-defined frequently used values for parsing strings and characters, and other common parser patterns.

### Strings and Characters

- `upper()`: parses any uppercase alphabetical character.
- `lower()`: parses any lowercase letter.
- `letter()`: parses a case-insensitive character of the alphabet.
- `digit()`: parses a numeric character.
- `symbol()`: parses one of a variety of non-alphanumeric characters.

### Lifting

- `lift_string()`: convert a character parser to a string parser.
- `lift_vector()`: convert to an `std::vector` of a value's type, containing a single element with the parser's result.

### Block Parsing

- `sep_by()`: parse a repeating sequence (one or more) of a parser value followed by a separator value.
- `end_by()`: behaves the same as `sep_by()`, however, the sequence must be terminated by the separator.
