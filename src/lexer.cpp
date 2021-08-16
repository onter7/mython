#include "lexer.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <sstream>
#include <unordered_map>

using namespace std;

namespace parse {

	static const std::unordered_map<std::string, Token> str_to_keywords{
		{ "class"s, token_type::Class{} },
		{ "return"s, token_type::Return{} },
		{ "if"s, token_type::If{} },
		{ "else"s, token_type::Else{} },
		{ "def"s, token_type::Def{} },
		{ "print"s, token_type::Print{} },
		{ "and"s, token_type::And{} },
		{ "or"s, token_type::Or{} },
		{ "not"s, token_type::Not{} },
		{ "None"s, token_type::None{} },
		{ "True"s, token_type::True{} },
		{ "False"s, token_type::False{} }
	};

	static const std::unordered_map<std::string, Token> str_to_comparison_lexems{
		{ "=="s, token_type::Eq{} },
		{ "!="s, token_type::NotEq{} },
		{ "<="s, token_type::LessOrEq{} },
		{ ">="s, token_type::GreaterOrEq{} }
	};

	bool operator==(const Token& lhs, const Token& rhs) {
		using namespace token_type;

		if (lhs.index() != rhs.index()) {
			return false;
		}
		if (lhs.Is<Char>()) {
			return lhs.As<Char>().value == rhs.As<Char>().value;
		}
		if (lhs.Is<Number>()) {
			return lhs.As<Number>().value == rhs.As<Number>().value;
		}
		if (lhs.Is<String>()) {
			return lhs.As<String>().value == rhs.As<String>().value;
		}
		if (lhs.Is<Id>()) {
			return lhs.As<Id>().value == rhs.As<Id>().value;
		}
		return true;
	}

	bool operator!=(const Token& lhs, const Token& rhs) {
		return !(lhs == rhs);
	}

	std::ostream& operator<<(std::ostream& os, const Token& rhs) {
		using namespace token_type;

#define VALUED_OUTPUT(type) \
    if (auto p = rhs.TryAs<type>()) return os << #type << '{' << p->value << '}';

		VALUED_OUTPUT(Number);
		VALUED_OUTPUT(Id);
		VALUED_OUTPUT(String);
		VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
    if (rhs.Is<type>()) return os << #type;

		UNVALUED_OUTPUT(Class);
		UNVALUED_OUTPUT(Return);
		UNVALUED_OUTPUT(If);
		UNVALUED_OUTPUT(Else);
		UNVALUED_OUTPUT(Def);
		UNVALUED_OUTPUT(Newline);
		UNVALUED_OUTPUT(Print);
		UNVALUED_OUTPUT(Indent);
		UNVALUED_OUTPUT(Dedent);
		UNVALUED_OUTPUT(And);
		UNVALUED_OUTPUT(Or);
		UNVALUED_OUTPUT(Not);
		UNVALUED_OUTPUT(Eq);
		UNVALUED_OUTPUT(NotEq);
		UNVALUED_OUTPUT(LessOrEq);
		UNVALUED_OUTPUT(GreaterOrEq);
		UNVALUED_OUTPUT(None);
		UNVALUED_OUTPUT(True);
		UNVALUED_OUTPUT(False);
		UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

		return os << "Unknown token :("sv;
	}

	Lexer::Lexer(std::istream& input)
		: input_(input) {
		indentation_levels_.push(0);
		NextToken();
	}

	const Token& Lexer::CurrentToken() const {
		return tokens_.back();
	}

	Token Lexer::NextToken() {
		SkipSpaces();
		const char c = input_.peek();
		Token token;
		if (current_line_indentation_ != indentation_levels_.top()) {
			token = ParseIndent();
		}
		else if (c == EOF) {
			if (!(tokens_.empty() ||
				CurrentToken().Is<token_type::Newline>() ||
				CurrentToken().Is<token_type::Eof>() ||
				CurrentToken().Is<token_type::Dedent>())) {
				token = token_type::Newline{};
			}
			else {
				token = token_type::Eof{};
			}
		}
		else if (c == '\n') {
			token = token_type::Newline{};
			input_.ignore();
		}
		else if (std::isdigit(c)) {
			token = ParseNumber();
		}
		else if (std::isalpha(c) || c == '_') {
			token = ParseIdentifier();
		}
		else if (c == '\'' || c == '\"') {
			token = ParseString();
		}
		else if (c == '!' || c == '<' || c == '>' || c == '=') {
			input_.ignore();
			const char next_c = input_.peek();
			if (next_c == '=') {
				std::stringstream ss;
				ss << c << next_c;
				token = str_to_comparison_lexems.at(ss.str());
				input_.ignore();
			}
			else {
				token = token_type::Char{ c };
			}
		}
		else {
			token = token_type::Char{ c };
			input_.ignore();
		}
		tokens_.push_back(token);
		return token;
	}

	token_type::Number Lexer::ParseNumber() {
		std::stringstream ss;
		while (std::isdigit(input_.peek())) {
			ss << static_cast<char>(input_.get());
		}
		return token_type::Number{ std::stoi(ss.str()) };
	}

	token_type::String Lexer::ParseString() {
		std::stringstream ss;
		const char start_symbol = input_.get();
		char c;
		while ((c = input_.get()) != start_symbol) {
			if (c == '\\') {
				const char next_c = input_.get();
				switch (next_c)
				{
				case '\'':
				case '\"':
					ss << next_c;
					break;
				case 'n':
					ss << '\n';
					break;
				case 't':
					ss << '\t';
					break;
				default:
					ss << c << next_c;
					break;
				}
			}
			else {
				ss << c;
			}
		}
		return token_type::String{ ss.str() };
	}

	Token Lexer::ParseIdentifier() {
		std::stringstream ss;
		for (char c = input_.peek(); std::isalpha(c) || std::isdigit(c) || c == '_'; c = input_.peek()) {
			ss << static_cast<char>(input_.get());
		}
		std::string identifier{ ss.str() };
		if (str_to_keywords.count(identifier)) {
			return str_to_keywords.at(identifier);
		}
		else {
			return token_type::Id{ identifier };
		}
	}

	Token Lexer::ParseIndent() {
		Token token;
		if (current_line_indentation_ > indentation_levels_.top()) {
			indentation_levels_.push(current_line_indentation_);
			token = token_type::Indent{};
		}
		else {
			indentation_levels_.pop();
			if (indentation_levels_.top() < current_line_indentation_) {
				throw LexerError("Unexpected indentation"s);
			}
			token = token_type::Dedent{};
		}
		return token;
	}

	void Lexer::SkipSpaces() {
		int skipped = 0;
		const bool is_new_line = !tokens_.empty() && CurrentToken().Is<token_type::Newline>();
		while (true) {
			while (input_.peek() == ' ') {
				input_.ignore();
				++skipped;
			}
			if (input_.peek() == '#') {
				while (input_.peek() != '\n' && input_.peek() != EOF) {
					input_.ignore();
				}
			}
			if (input_.peek() != '\n' || !(tokens_.empty() || CurrentToken().Is<token_type::Newline>())) {
				if (is_new_line && skipped != indentation_levels_.top()) {
					if (skipped % 2 != 0) {
						throw LexerError("Unexpected indenation"s);
					}
					current_line_indentation_ = skipped;
				}
				break;
			}
			skipped = 0;
			input_.ignore();
		}
	}

}  // namespace parse