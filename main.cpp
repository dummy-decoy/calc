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
name ::= letter+ (digit|letter)*
identifier ::= name ('(' (expr (',' expr)*)? ')')?
primary ::= number|identifier|('(' expr ')')
factor ::= primary ('^' primary)?
term ::= factor (('*'|'/'|'%') factor)*
expr ::= ('+'|'-')? term (('+'|'-') term)*
*/

using value_t = double;

std::map<std::string, value_t> constants;
std::map<std::string, value_t> variables;


void trace(std::string msg, char token) {
    std::cout << "trace: " << msg << " (" << token << ")" << std::endl;
}


class input_t {
    public:
        input_t(std::istream& input) : input_(input){
            advance();
        }
        bool eof() {
            return input_.eof();
        }
        char next() {
            return next_;
        }
        void advance(){
            next_ = input_.get();
        }
        void skip() {
            while (!eof() && isblank(next()))
                advance();
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
    if (input.eof() || !isdigit(chr))
        throw std::invalid_argument("number: expected digit, got "+chr);
    while (!input.eof() && isdigit(chr)) {
        result = result*10 + static_cast<int>(chr-'0');
        input.advance();
        chr = input.next();
    }
    if (!input.eof() && (chr == '.')) {
        input.advance();
        chr = input.next();
        if (input.eof() || !isdigit(chr))
            throw std::invalid_argument("number: expected digit after decimal point, got "+chr);
        while (!input.eof() && isdigit(chr)) {
            result = result*10 + static_cast<int>(chr-'0');
            exp--;
            input.advance();
            chr = input.next();
        }
        result *= pow(10,exp);
        exp = 0;
    }
    if (!input.eof() && (chr == 'e')) {
        input.advance();
        chr = input.next();
        if (chr == '+') {
            input.advance();
        } else if (chr == '-') {
            negexp = true;
            input.advance();
        } else if (input.eof() || !isdigit(chr))
            throw std::invalid_argument("number: expected sign or digit after exponent indicator, got "+chr);
        chr = input.next();
        while (!input.eof() && isdigit(chr)) {
            exp = exp*10 + static_cast<int>(chr-'0');
            input.advance();
            chr = input.next();
        }
        if (negexp)
            exp = -exp;
        result *= pow(10, exp);
    }
    input.skip();
    return result;
}
value_t parse_identifier(input_t& input) {
    std::string name;

    char chr = input.next();
    if (input.eof() || !isalpha(chr))
        throw std::invalid_argument("identifier: expected alpha, got "+chr);
    while (!input.eof() && (isalnum(chr) || (chr == '_'))) {
        name.push_back(chr);
        input.advance();
        chr = input.next();
    }

    if (!input.eof() && (chr == '(')) {
        throw std::runtime_error("function calls are not yet implemented");
    } else {
        decltype(constants)::iterator constant = constants.find(name);
        if (constant != constants.end())
            return constant->second;
        decltype(variables)::iterator variable = variables.find(name);
        if (variable != variables.end()) 
            return variable->second;
        throw std::runtime_error("undefined identifier: "+name);
    }
    input.skip();
}
value_t parse_primary(input_t& input) {
    value_t result = 0;
    char chr = input.next();
    if (input.eof()) {
        throw std::invalid_argument("primary: input is empty");
    } else if (isdigit(chr)) {
        result = parse_number(input);
    } else if (isalpha(chr)) {
        result = parse_identifier(input);
    }  else if (chr == '(') {
        input.advance(); 
        result = parse_expr(input);
        input.skip();
        chr = input.next();
        if (input.eof() || (chr != ')'))
            throw std::invalid_argument("primary: expected ')', got "+chr);
        input.advance();
        input.skip();
    } else {
        throw std::invalid_argument("primary: expected number, identifier or (expression)");
    }
    return result;
}
value_t parse_factor(input_t& input) {
    value_t result = parse_primary(input);
    char chr = input.next();
    if (!input.eof() && (chr == '^')) {
        input.advance();
        input.skip();
        result = pow(result, parse_primary(input)); 
    }
    return result;
}
value_t parse_term(input_t& input) {
    value_t result = parse_factor(input);
    char chr = input.next();
    while (!input.eof() && ((chr == '*') || (chr == '/') || (chr == '%'))) {
        if (chr == '*') {
            input.advance();
            input.skip();
            result *= parse_factor(input);
        } else if (chr == '/') {
            input.advance();
            input.skip();
            result /= parse_factor(input);
        } else if (chr == '%') {
            input.advance();
            input.skip();
            result  = fmod(result, parse_factor(input));
        }
        chr = input.next();
    }
    return result;
}
value_t parse_expr(input_t& input) {
    bool negate = false;

    input.skip();
    char chr = input.next();
    if (input.eof()) {
        throw std::invalid_argument("expr: input is empty");
    } else if (chr == '+') {
        input.advance();
        input.skip();
    } else if (chr == '-') {
        negate = true;
        input.advance();
        input.skip();
    }

    value_t result = parse_term(input);
    if (negate)
        result = -result;
    
    chr = input.next();
    while (!input.eof() && ((chr == '+') || (chr == '-'))) {
        if (chr == '+') {
            input.advance();
            input.skip();
            result += parse_term(input);
        } else if (chr == '-') {
            input.advance();
            input.skip();
            result -= parse_term(input);
        }
        chr = input.next();
    }

    return result;
}

void setup() {
    constants["pi"] = 3.1415926535898;
    constants["e"]  = 2.7182818284590;
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
            std::cout << "parse error: " << error.what() << std::endl;
        } catch (const std::runtime_error& error) {
            std::cout << "execution error: " << error.what() << std::endl;
        }
    }
}
