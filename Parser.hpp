#pragma once 

#include "Lexer.hpp"
#include <cmath>
#include <cstdlib>
#include <functional>
#include <limits>
#include <string>

// Needs to turn the Lexer token in std::function<float(float)>
class Parser {

public:
	Parser(const char* function) : m_lexer(function) {
		Lexer_next();
	}

	void Lexer_next() {
		function_token = m_lexer.next();
	}

	std::function<float(float)> parse() {
		SkipFunctionHeader();
		std::function<float(float)> value = ParseExpression();
		return value;
	}

	void SkipFunctionHeader() {
		// Skips "f(x) = "
		if (!function_token.is(Token::Kind::Identifier)) return;

		// Just So it doesn't skip the first letter while not writing the body
		Token saved = function_token;
		Lexer_next();

		if (!function_token.is(Token::Kind::LeftParen)) { function_token = saved; return; }
		Lexer_next();

		if (!function_token.is(Token::Kind::Identifier)) { function_token = saved; return; }
		Lexer_next();

		if (!function_token.is(Token::Kind::RightParen)) { function_token = saved; return; }
		Lexer_next();

		if (!function_token.is(Token::Kind::Equal)) { function_token = saved; return; }
		Lexer_next();
	}

	std::function<float(float)> ParseExpression() {

		std::function<float(float)> left = ParseTerm();

		for (;;) {
			if (function_token.is(Token::Kind::Plus)) {
				Lexer_next();
				std::function<float(float)> right = ParseTerm();
				left = [l = left, right](float x) { return l(x) + right(x); };
			}
			else if (function_token.is(Token::Kind::Minus)) {
				Lexer_next();
				std::function<float(float)> right = ParseTerm();
				left = [l = left, right](float x) { return l(x) - right(x); };
			}
			else {
				break;
			}
		}

		return left;
	}

	std::function<float(float)> ParseTerm() {

		std::function<float(float)> left = ParsePrimary();

		for (;;) {
			if (function_token.is(Token::Kind::Asterisk)) {
				Lexer_next();
				std::function<float(float)> right = ParsePrimary();
				left = [l = left, right](float x) { return l(x) * right(x); };
			}
			else if (function_token.is(Token::Kind::Slash)) {
				Lexer_next();
				std::function<float(float)> right = ParsePrimary();
				left = [l = left, right](float x) {
					float d = right(x);
					return d != 0.0f ? l(x) / d : std::numeric_limits<float>::quiet_NaN();
					};
			}
			else {
				break;
			}
		}

		return left;
	}

	std::function<float(float)> ParsePrimary() {

		if (function_token.is(Token::Kind::Minus))
		{
			Lexer_next();
			auto operand = ParsePrimary();
			return [o = operand](float x) { return -o(x); };
		}

		if (function_token.is(Token::Kind::Number)) {
			std::string value(function_token.lexeme());
			Lexer_next();

			if (function_token.is(Token::Kind::Dot)) { // For Decimal Value Support 
				Lexer_next();
				value += '.';
				if (function_token.is(Token::Kind::Number)) {
					value += std::string(function_token.lexeme());
					Lexer_next();
				}
			}
			// stof is recommended way of changing string to float 
			float number_value = std::stof(value);
			return [number_value](float) { return number_value; };
		}

		if (function_token.is(Token::Kind::Identifier)) {
			std::string name(function_token.lexeme());
			Lexer_next();

			// For Math things
			if (function_token.is(Token::Kind::LeftParen)) {
				Lexer_next();
				auto arg = ParseExpression();
				if (function_token.is(Token::Kind::RightParen)) Lexer_next();

				if (name == "sin")  return [a = arg](float x) { return std::sin(a(x));   };
				if (name == "cos")  return [a = arg](float x) { return std::cos(a(x));   };
				if (name == "tan")  return [a = arg](float x) { return std::tan(a(x));   };
				if (name == "log")  return [a = arg](float x) { return std::log10(a(x)); };
				if (name == "ln")   return [a = arg](float x) { return std::log(a(x));   };
				if (name == "sqrt") return [a = arg](float x) { return std::sqrt(a(x));  };
				if (name == "abs")  return [a = arg](float x) { return std::abs(a(x));   };
				if (name == "exp")   return [a = arg](float x) { return std::exp(a(x));  };
				if (name == "floor") return [a = arg](float x) { return std::floor(a(x));};
				if (name == "ceil")  return [a = arg](float x) { return std::ceil(a(x)); };
				if (name == "round") return [a = arg](float x) { return std::round(a(x));};
				if (name == "sign")  return [a = arg](float x) { return (a(x) > 0.f) ? 1.f : (a(x) < 0.f) ? -1.f : 0.f; };

				return arg; // unknown function, just pass through its argument
			}
			if (name == "pi") return [](float) { return  3.14159265358979323846f;  };
			if (name == "e")  return [](float) { return 2.71828182845904523536f;   };

			return [](float x) { return x; };
		}

		if (function_token.is(Token::Kind::LeftParen))
		{
			Lexer_next();
			auto arg = ParseExpression();
			if (function_token.is(Token::Kind::RightParen)) Lexer_next();
			return arg;
		}

		return [](float) { return 0.f; };
	}

private:
	const char* m_function;
	Lexer m_lexer;
	// Default function_token Kind ( Just for First time )
	Token function_token{ Token::Kind::Unexpected };

};