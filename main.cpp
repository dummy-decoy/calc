#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <cctype>
#include <functional>
#include <stdexcept>
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <limits>

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
statement ::= expr ('>'  name)
*/

using value_t = double;
using function_t = std::function<value_t(const std::vector<value_t>&)>;

std::map<std::string, value_t> constants;
std::map<std::string, value_t> variables;
std::map<std::string, function_t> functions;

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
        void ignore() {
            while (!eof() && (next() != '\n'))
                advance();
        }
    private:
        std::istream& input_;
        char next_;
};

value_t parse_expr(input_t& input);

value_t parse_number(input_t& input) {
    value_t result = 0;
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
std::string parse_name(input_t& input) {
    std::string name;

    char chr = input.next();
    if (input.eof() || !isalpha(chr))
        throw std::invalid_argument("name: expected alpha, got "+chr);
    while (!input.eof() && (isalnum(chr) || (chr == '_'))) {
        name.push_back(chr);
        input.advance();
        chr = input.next();
    }
    input.skip();
    return name;
}
value_t parse_identifier(input_t& input) {
    std::string name = parse_name(input);

    char chr = input.next();
    if (!input.eof() && (chr == '(')) {
        std::vector<value_t> args;
        input.advance();
        input.skip();
        chr = input.next();
        if (!input.eof() && (chr != ')')) {
            args.push_back(parse_expr(input));
            chr = input.next();
            while (!input.eof() && (chr == ',')) {
                input.advance();
                input.skip();
                args.push_back(parse_expr(input));
                chr = input.next();
            }
        }
        if (!input.eof() && (chr != ')')) 
            throw std::invalid_argument("call: expected ), got "+chr);
        input.advance();
        input.skip();

        decltype(functions)::iterator function = functions.find(name);
        if (function != functions.end())
            return function->second(args);
        throw std::runtime_error("identifier: undefined function: "+name);
    } else {
        decltype(constants)::iterator constant = constants.find(name);
        if (constant != constants.end())
            return constant->second;
        decltype(variables)::iterator variable = variables.find(name);
        if (variable != variables.end()) 
            return variable->second;
        throw std::runtime_error("identifier: undefined identifier: "+name);
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
value_t parse_statement(input_t& input) {
    input.skip();
    value_t result = parse_expr(input);
    char chr = input.next();
    if (!input.eof() && (chr == '>')) {
        input.advance();
        input.skip();
        
        std::string name = parse_name(input);
        if (constants.find(name) != constants.end())
            throw std::invalid_argument("statement: cannot assign value to constant");
        if (functions.find(name) != functions.end())
            throw std::invalid_argument("statement: cannot assign value to function");
        variables[name] = result;   
    }
    return result;
}

void setup() {
    constants["pi"] = 3.1415926535898;
    constants["e"]  = 2.7182818284590;
    functions["abs"] = [](std::vector<value_t> args)->value_t { 
        if (args.size()!=1)
            throw std::runtime_error("abs: wrong number of arguments"); 
        return abs(args[0]); 
    };
    functions["pow"] = [](std::vector<value_t> args)->value_t { 
        if (args.size()!=2)
            throw std::runtime_error("sqrt: wrong number of arguments"); 
        return pow(args[0], args[1]); 
    };
    functions["sqrt"] = [](std::vector<value_t> args)->value_t { 
        if (args.size()!=1)
            throw std::runtime_error("sqrt: wrong number of arguments"); 
        return sqrt(args[0]); 
    };
    functions["exp"] = [](std::vector<value_t> args)->value_t { 
        if (args.size()!=1)
            throw std::runtime_error("exp: wrong number of arguments"); 
        return exp(args[0]); 
    };
    functions["log"] = [](std::vector<value_t> args)->value_t { 
        if (args.size() == 1)
            return log(args[0]);
        else if (args.size() == 2)
            return log(args[0])/log(args[1]);
        else
            throw std::runtime_error("log: wrong number of arguments"); 
    };
    functions["sin"] = [](std::vector<value_t> args)->value_t { 
        if (args.size()!=1)
            throw std::runtime_error("sin: wrong number of arguments"); 
        return sin(args[0]); 
    };
    functions["cos"] = [](std::vector<value_t> args)->value_t { 
        if (args.size()!=1) 
            throw std::runtime_error("cos: wrong number of arguments"); 
        return cos(args[0]); 
    };
    functions["tan"] = [](std::vector<value_t> args)->value_t { 
        if (args.size()!=1) 
            throw std::runtime_error("tan: wrong number of arguments"); 
        return tan(args[0]); 
    };
}
int main(int argc, char*argv[]) {
    setup();
    while (true) {
        std::cout << "? " << std::flush;
        input_t input(std::cin);
        while (input.next() != '\n') {
            try {
                value_t result = parse_statement(input);
                std::cout << "= " << result << std::endl;
            } catch (const std::invalid_argument& error) {
                std::cout << "parse error: " << error.what() << std::endl;
                input.ignore();
            } catch (const std::runtime_error& error) {
                std::cout << "execution error: " << error.what() << std::endl;
                input.ignore();
            }
        }
    }
}
