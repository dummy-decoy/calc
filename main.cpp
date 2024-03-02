#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <cctype>
#include <stdexcept>
#include <map>
#include <string>
#include <iostream>


/*
digit = ('0'|'1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9')
letter = ('a'..'z'|'A'..'Z'|'_')

number ::= digit+ ('.' digit+)? ('e' ('+'|'-')? digit+)?
identifier ::= letter+ (digit|letter)*
primary ::= number|identifier|('(' expr ')')
factor ::= primary ('^' primary)?
term ::= factor (('*'|'/'|'%') factor)*
expr ::= ('+'|'-')? term (('+'|'-') term)*
*/

using value_t = double;
std::map<std::string, value_t> env;


void trace(std::string msg, char token) {
    std::cout << "trace: " << msg << " (" << token << ")" << std::endl;
}


class input_t {
    public:
        input_t(std::istream& input) : input_(input){
            advance();
        }
        char next() {
            return next_;
        }
        void advance(){
            next_ = input_.get();
        }
    private:
        std::istream& input_;
        char next_;
};

value_t parse_expr(input_t& input);

value_t parse_number(input_t& input) {
    value_t result;
    int exp = 0;
    bool negexp = false;

    char chr = input.next();
    while (isdigit(chr)) {
        result = result*10 + static_cast<int>(chr-'0');
        input.advance();
        chr = input.next();
    }
    if (chr == '.') {
        input.advance();
        chr = input.next();
        while (isdigit(chr)) {
            result = result*10 + static_cast<int>(chr-'0');
            exp--;
            input.advance();
            chr = input.next();
        }
        result *= pow(10,exp);
        exp = 0;
    }
    if (chr == 'e') {
        input.advance();
        chr = input.next();
        if (chr == '+') {
            input.advance();
        } else if (chr == '-') {
            negexp = true;
            input.advance();
        }
        chr = input.next();
        while (isdigit(chr)) {
            exp = exp*10 + static_cast<int>(chr-'0');
            input.advance();
            chr = input.next();
        }
        if (negexp)
            exp = -exp;
        result *= pow(10, exp);
    }
    return result;
}
value_t parse_identifier(input_t& input) {
    std::string name;

    char chr = input.next();
    if (!isalpha(chr))
        throw std::invalid_argument("parse error: identifier: expected alpha, got "+chr);
    while (isalnum(chr) || (chr == '_')) {
        name.push_back(chr);
        input.advance();
        chr = input.next();
    }

    decltype(env)::iterator ident = env.find(name);
    if (ident == env.end())
        throw std::invalid_argument("execution error: unknown identifier: "+name);
    
    return ident->second;
}
value_t parse_primary(input_t& input) {
    value_t result;
    char chr = input.next();
    if (isdigit(chr)) {
        result = parse_number(input);
    } else if (isalpha(chr)) {
        result = parse_identifier(input);
    }  else if (chr == '(') {
        input.advance();
        result = parse_expr(input);
        chr = input.next();
        if (chr != ')')
            throw std::invalid_argument("parse error: primary: expected ')', got "+chr);
        input.advance();
    }
    return result;
}
value_t parse_factor(input_t& input) {
    value_t result = parse_primary(input);

    char chr = input.next();
    if (chr == '^') {
        input.advance();
        result = pow(result, parse_primary(input)); 
    }
    return result;
}
value_t parse_term(input_t& input) {
    value_t result = parse_factor(input);

    char chr = input.next();
    while ((chr == '*') || (chr == '/') || (chr == '%')) {
        if (chr == '*') {
            input.advance();
            result *= parse_factor(input);
        } else if (chr == '/') {
            input.advance();
            result /= parse_factor(input);
        } else if (chr == '%') {
            input.advance();
            result  = fmod(result, parse_factor(input));
        }
        chr = input.next();
    }
    return result;
}
value_t parse_expr(input_t& input) {
    bool negate = false;

    char chr = input.next();
    if (chr == '+') {
        input.advance();
    } else if (chr == '-') {
        negate = true;
        input.advance();
    }

    value_t result = parse_term(input);
    if (negate)
        result = -result;
    
    chr = input.next();
    while ((chr == '+') || (chr == '-')) {
        if (chr == '+') {
            input.advance();
            result += parse_term(input);
        } else if (chr == '-') {
            input.advance();
            result -= parse_term(input);
        }
        chr = input.next();
    }

    return result;
}


void setup() {
    env["pi"] = 3.1415926535898;
}

int main(int argc, char*argv[]) {
    setup();
    while (true) {
        std::cout << "? " << std::flush;
        try {
            input_t input(std::cin);
            value_t result = parse_expr(input);
            std::cout << "= " << result << std::endl;
        } catch (const std::invalid_argument& error) {
            std::cout << error.what() << std::endl;
        }
    }
}
