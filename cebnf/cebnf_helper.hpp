#pragma once

#include "cebnf.hpp"

namespace cebnf {

	enum cebnf_expanded_type {
		expand_begin = cebnf_type::base_end,
		digit,
		digit_excluding_zero,
		natural_number,
		integer,
		real_number,
		alphabet_s,
		alphabet_l,
		alphabet,
		left_enclosure,//left surrounding term ( { [ " ' etc...
		right_enclosure,//right surrounding term ) } ] " ' etc...
		string,
		string_in_enclosure,//not replacement escape sequence series (example: \\\" ->  \\\")
		string_in_enclosure_2,//replacement escape sequence series (example: \\\" ->  \")
		expand_end
	};

	/*expanded value class*/
	struct Digit;
	struct DigitExcludingZero;
	struct NaturalNumber;
	struct Integer;
	struct RealNumber;

	struct AlphabetS;
	struct AlphabetL;
	struct Alphabet;
	struct StringIE;
	struct StringIE2;

	/*-------------------------------------------------------------------------------*/

	/*
	one digit term
	- Digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"; 
	*/
	struct Digit : public CEBNFBase, tagValue {

		std::size_t TokenID = cebnf_expanded_type::digit;
		std::string _s = "";

		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {

			std::size_t itr = begin;
			std::string _s = "";

			if ('0' <= s[itr] && s[itr] <= '9') {
				_s += s[itr];
				return Tree::Create(TokenID, _s, _s.length());
			}
			else {
				return Tree::Create();
			}

		}
	};

	/*
	one digit term excluding zero
	- DigitExcludingZero = "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9";
	*/
	struct DigitExcludingZero : public CEBNFBase, tagValue {

		std::size_t TokenID = cebnf_expanded_type::digit_excluding_zero;
		std::string _s = "";

		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {
			std::size_t itr = begin;
			std::string _s = "";

			if ('1' <= s[itr] && s[itr] <= '9') {
				_s += s[itr];
				return Tree::Create(TokenID, _s, _s.length());
			}
			else {
				return Tree::Create();
			}

		}
	};

	/*
	natural number
	- NaturalNumber = DigitExcludingZero , { Digit };
	*/
	struct NaturalNumber : public CEBNFBase, tagValue {

		std::size_t TokenID = cebnf_expanded_type::natural_number;
		std::string _s = "";

		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {

			std::size_t itr = begin;
			std::string _s = "";

			if ('1' <= s[itr] && s[itr] <= '9') {
				_s += s[itr++];
			}
			else {
				return Tree::Create();
			}

			while (true) {

				if ('0' <= s[itr] && s[itr] <= '9') {
					_s += s[itr++];
				}
				else {
					return Tree::Create(TokenID, _s, _s.length());
				}
			}

		}
	};
	
	/*
	integer number
	- Integer = [ + | - ] , ( "0" | NaturalNumber );
	*/
	struct Integer : public CEBNFBase, tagValue {

		std::size_t TokenID = cebnf_expanded_type::integer;
		std::string _s = "";

		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {

			std::size_t itr = begin;
			std::string _s = "";

			if (s[itr] == '+' || s[itr] == '-') {
				_s += s[itr++];
			}

			if (s[itr] == '0') {
				_s += s[itr];
				return Tree::Create(TokenID, _s, _s.length());
			}else if('1' <= s[itr] && s[itr] <= '9'){
				_s += s[itr++];

				while (true) {

					if ('0' <= s[itr] && s[itr] <= '9') {
						_s += s[itr++];
					}
					else {
						return Tree::Create(TokenID, _s, _s.length());
					}
				}
			}
			else {
				return Tree::Create();
			}

		}
	};

	/*
	real number
	- RealNumber = Integer , "." , { Digit } | [ "+" | "-" ] , "." , Digit , { Digit } ;
	*/
	struct RealNumber : public CEBNFBase, tagValue {

		std::size_t TokenID = cebnf_expanded_type::real_number;
		std::string _s = "";

		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {

			std::size_t itr = begin;
			std::string _s = "";
			bool exist_integer_area = true;
			bool exist_after_the_decimal_point = true;

			/*integer area*/

			if (s[itr] == '+' || s[itr] == '-') {
				_s += s[itr++];
			}

			if (s[itr] == '0') {
				_s += s[itr++];
				//return Tree::Create(TokenID, _s, _s.length());
			}
			else if ('1' <= s[itr] && s[itr] <= '9') {
				_s += s[itr++];

				while (true) {

					if ('0' <= s[itr] && s[itr] <= '9') {
						_s += s[itr++];
					}
					else {
						break;
						//return Tree::Create(TokenID, _s, _s.length());
					}
				}
			}
			else {
				exist_integer_area = false;
			}

			/* must exist the decimal point. */

			if (s[itr] == '.') {
				_s += s[itr++];
			}
			else {
				return Tree::Create();
			}

			/*after the decimal point*/

			if ('0' <= s[itr] && s[itr] <= '9') {
				_s += s[itr++];

				while (true) {
					if ('0' <= s[itr] && s[itr] <= '9') {
						_s += s[itr++];
						//return Tree::Create(TokenID, _s, _s.length());
					}
					else {
						break;
						//return Tree::Create();
					}
				}
			}
			else {
				exist_after_the_decimal_point = false;
			}

			/*finish*/

			if (exist_integer_area || exist_after_the_decimal_point) {
				return Tree::Create(TokenID, _s, _s.length());
			}
			else {
				return Tree::Create();
			}



		}
	};

	/*-------------------------------------------------------------------------------*/

	struct StringHelper {

		static std::string stringHexWard() {
			return u8"0123456789abcdefABCDEF";
		}

		/*input second character :: \x -> changeEscapeSequence('x'), return std::string("\x")*/
		static std::string changeEscapeSequence(const char c){
			switch (c) {
			case '\"': return std::string("\"");
			case '\'': return std::string("\'");
			case 'n' : return std::string("\n");
			case '\\': return std::string("\\");
			case '\?': return std::string("\?");
			case 't' : return std::string("\t");
			case 'b' : return std::string("\b");
			case 'r' : return std::string("\r");
			case 'f' : return std::string("\f");
			case 'v' : return std::string("\v");
			case 'a' : return std::string("\a");
			default  : return std::string("");
			}
		}

		/* Unicode codepoint to UTF-8
		cp 00000000 0xxxxxxx => utf8 0xxxxxxx
		cp 00000yyy yyxxxxxx => utf8 110yyyyy 10xxxxxx
		cp zzzzyyyy yyxxxxxx => utf8 1110zzzz 10yyyyyy 10xxxxxx
		cp 000uuuzz zzzzyyyy yyxxxxxx => 11110uuu 10zzzzzz 10yyyyyy 10xxxxxx
		*/
		static std::string encodeUTF8(uint32_t codepoint) {

			if (0x10FFFF < codepoint) {
				return std::string();
			}

			std::string dst;
			if (codepoint < 0x0080) {
				dst += static_cast<char>(codepoint);
				return dst;
			}
			else if (codepoint < 0x0800) {
				dst += static_cast<char>((codepoint >> 6) | 0xc0);
				dst += static_cast<char>((codepoint & 0x3f) | 0x80);
				return dst;
			}
			else if (codepoint < 0x10000) {
				dst += static_cast<char>((codepoint >> 12) | 0xe0);
				dst += static_cast<char>(((codepoint >> 6) & 0x3f) | 0x80);
				dst += static_cast<char>((codepoint & 0x3f) | 0x80);
				return dst;
			}
			else {
				dst += static_cast<char>((codepoint >> 18) | 0xf0);
				dst += static_cast<char>(((codepoint >> 12) & 0x3f) | 0x80);
				dst += static_cast<char>(((codepoint >> 6) & 0x3f) | 0x80);
				dst += static_cast<char>((codepoint & 0x3f) | 0x80);
			}
			return dst;
		}

		static bool isSurrogatePair_High(uint32_t codepoint) {
			return (0xD800 <= codepoint && codepoint <= 0xDBFF) ? true : false;
		}
		static bool isSurrogatePair_Low(uint32_t codepoint) {
			return (0xDC00 <= codepoint && codepoint <= 0xDFFF) ? true : false;
		}

		static uint32_t surrogate2codepoint(uint32_t high, uint32_t low) {
			return 0x10000 + (high - 0xD800) * 0x400 + (low - 0xDC00);
		}
		static std::array<uint32_t, 2> codepoint2surrogate(uint32_t codepoint) {
			return std::array<uint32_t, 2>{(codepoint - 0x10000) / 0x400 + 0xD800, (codepoint - 0x10000) % 0x400 + 0xDC00};
		}

		/* X ... XXXXXX < 10FFFF
		return UTF8 string and loading character size
		surplus characters are Ignored*/
		static std::pair<std::string, size_t> toUTF8(std::string hex_string) {
			size_t check = hex_string.find_first_not_of(StringHelper::stringHexWard());
			uint32_t codepoint = std::stoul(hex_string.substr(0, check), nullptr, 16);

			std::string UTF8 = encodeUTF8(codepoint);
			return std::pair<std::string, size_t>(UTF8, check);
		}

		/* XXXX + (\uXXXX : surrogate pair)
		return UTF8 string and loading character size
		surplus characters are Ignored*/
		static std::pair<std::string, size_t> toUTF8surrogate(std::string hex_string) {
			size_t check = hex_string.find_first_not_of(StringHelper::stringHexWard());
			
			if (check < 4) {
				/*error*/
				return std::pair<std::string, size_t>(std::string(), check);
			}

			/* later, check >= 4 and use length is 4 */

			uint32_t codepoint = std::stoul(hex_string.substr(0, 4), nullptr, 16);

			/*if surrogate pair*/
			if (isSurrogatePair_High(codepoint)) {
				if (hex_string.length() != 10) {
					/*error*/
					return std::pair<std::string, size_t>(std::string(), 4);
				}

				if (hex_string[4] != '\\' || hex_string[5] != 'u') {
					/*error*/
					return std::pair<std::string, size_t>(std::string(), 4);
				}

				size_t check2 = hex_string.substr(6).find_first_not_of(StringHelper::stringHexWard());

				if (check2 < 4) {
					/*error*/
					return std::pair<std::string, size_t>(std::string(), 4 + 2 + check2);
				}

				/* later, check2 >= 4 and use length is 4 */

				uint32_t codepoint2 = std::stoul(hex_string.substr(6, 4), nullptr, 16);

				if (isSurrogatePair_Low(codepoint2)) {
					std::string UTF8 = encodeUTF8(surrogate2codepoint(codepoint, codepoint2));
					return std::pair<std::string, size_t>(UTF8, 4 + 2 + 4);
				}
				else {
					/*error*/
					return std::pair<std::string, size_t>(std::string(), 4 + 2 + 4);
				}
			}
			/*not surrogate pair*/
			else {
				std::string UTF8 = encodeUTF8(codepoint);
				return std::pair<std::string, size_t>(UTF8, 4);
			}

		}

	};


	/*
	one small alphabet
	- AlphabetS = "a" | "b" | "c" | "d" | ... | "w" | "x" | "y" | "z";
	*/
	struct AlphabetS : public CEBNFBase, tagValue {

		std::size_t TokenID = cebnf_expanded_type::alphabet_s;
		std::string _s = "";

		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {

			std::size_t itr = begin;
			std::string _s = "";

			if ('a' <= s[itr] && s[itr] <= 'z') {
				_s += s[itr];
				return Tree::Create(TokenID, _s, _s.length());
			}
			else {
				return Tree::Create();
			}

		}
	};

	/*
	one large alphabet
	- AlphabetL = "A" | "B" | "C" | "D" | ... | "W" | "X" | "Y" | "Z";
	*/
	struct AlphabetL : public CEBNFBase, tagValue {

		std::size_t TokenID = cebnf_expanded_type::alphabet_l;
		std::string _s = "";

		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {

			std::size_t itr = begin;
			std::string _s = "";

			if ('A' <= s[itr] && s[itr] <= 'Z') {
				_s += s[itr];
				return Tree::Create(TokenID, _s, _s.length());
			}
			else {
				return Tree::Create();
			}

		}
	};

	/*
	one large or small alphabet
	- Alphabet = AlphabetL | AlphabetS;
	*/
	struct Alphabet : public CEBNFBase, tagValue {

		std::size_t TokenID = cebnf_expanded_type::alphabet;
		std::string _s = "";

		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {

			std::size_t itr = begin;
			std::string _s = "";

			if (('A' <= s[itr] && s[itr] <= 'Z') || ('a' <= s[itr] && s[itr] <= 'z')) {
				_s += s[itr];
				return Tree::Create(TokenID, _s, _s.length());
			}
			else {
				return Tree::Create();
			}

		}
	};


	/*string in brackets :: {string}, "string", 'string', ...
	- not replace escape sequence series (example: \\\" ->  \\\")
	*/
	struct StringIE : public CEBNFBase, tagValue {

		std::size_t TokenID = cebnf_expanded_type::string_in_enclosure;
		std::string _s = "";
		char _left_enclosure;
		char _right_enclosure;

		StringIE(const char left_enclosure, const char right_enclosure) : _left_enclosure(left_enclosure), _right_enclosure(right_enclosure) {}

		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {

			std::size_t itr = begin;
			std::string _s = "";
			std::string _l; _l += _left_enclosure;
			std::string _r; _r += _right_enclosure;

			if (s[itr] == _left_enclosure) {
				//_s += s[itr++];
				itr++;
			}
			else {
				return Tree::Create();
			}

			while (true) {

				if (s[itr] == _right_enclosure) {
					//_s += s[itr++];
					auto&& ldq = Tree::Create(cebnf_expanded_type::left_enclosure, _l, 1);
					auto&& main = Tree::Create(cebnf_expanded_type::string, _s, _s.length());
					auto&& rdq = Tree::Create(cebnf_expanded_type::right_enclosure, _r, 1);

					auto&& I = Tree::Create(TokenID, _l + _s + _r, _s.length() + 2);
					I.first[0]->mergeChildren(std::move(ldq.first));
					I.first[0]->mergeChildren(std::move(main.first));
					I.first[0]->mergeChildren(std::move(rdq.first));

					return std::move(I);
				}
				else if (s[itr] == '\0') {
					return Tree::Create();
				}
				else {
					_s += s[itr++];
				}

			}

		}
	};


	/*string in enclosure :: {string}, "string", 'string', ...
	- replace escape sequence series (example: \\\" ->  \")
	- replace Unicode codepoint to UTF-8
	*/
	struct StringIE2 : public CEBNFBase, tagValue {

		std::size_t TokenID = cebnf_expanded_type::string_in_enclosure_2;
		std::string _s = "";
		char _left_enclosure;
		char _right_enclosure;

		StringIE2(const char left_enclosure, const char right_enclosure) : _left_enclosure(left_enclosure), _right_enclosure(right_enclosure) {}

		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {

			std::size_t itr = begin;
			std::string _s = "";
			std::string _l; _l += _left_enclosure;
			std::string _r; _r += _right_enclosure;

			if (s[itr] == _left_enclosure) {
				//_s += s[itr++];
				itr++;
			}
			else {
				return Tree::Create();
			}

			while (true) {

				if (s[itr] == _right_enclosure) {
					//_s += s[itr++];
					auto&& ldq = Tree::Create(cebnf_expanded_type::left_enclosure, _l, 1);
					auto&& main = Tree::Create(cebnf_expanded_type::string, _s, _s.length());
					auto&& rdq = Tree::Create(cebnf_expanded_type::right_enclosure, _r, 1);

					auto&& I = Tree::Create(TokenID, _l + _s + _r, itr - begin + 1);
					I.first[0]->mergeChildren(std::move(ldq.first));
					I.first[0]->mergeChildren(std::move(main.first));
					I.first[0]->mergeChildren(std::move(rdq.first));

					return std::move(I);
				}
				else if (s[itr] == '\0') {
					return Tree::Create();
				}
				else if (s[itr] == '\\') {
					itr++;
					if (s[itr] == 'n' || s[itr] == '\'' || s[itr] == '\"' || s[itr] == '\\' || s[itr] == '\?' || s[itr] == 't' || s[itr] == 'b' || s[itr] == 'r' || s[itr] == 'f' || s[itr] == 'v' || s[itr] == 'a') {
						//_s += '\\';
						_s += StringHelper::changeEscapeSequence(/* \\ + */ s[itr]);
						itr++;
					}
					else if (s[itr] == 'u') {
						itr++;
						std::string subs = s.substr(itr, std::min(10U, s.length() - itr));
						auto utf8 = StringHelper::toUTF8surrogate(subs);
						if (utf8.first == std::string()) {
							return Tree::Create();
						}

						_s += utf8.first;
						itr += utf8.second;
					}
					else{
						return Tree::Create();
					}

				}
				else {
					_s += s[itr++];
				}

			}

		}
	};


}

