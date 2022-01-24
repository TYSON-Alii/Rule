#include <iostream>
#include <vector>
#include <string>
#include <sstream>
const auto& code = R"(
#include <aaaaa>
#include "bbbbb"
#define meraba
typedef Int int;

/*
selam
*/

// meraba
int main() {

	return 0;
};
)";
using namespace std;
class Parser {
private:
	vector<pair<string, string>> types;
	stringstream code;
public:
	inline Parser(const string& _code) { code << _code; };
	auto clear_comments() {
		string new_code;
		char c;
		bool multi_begin = false;
		bool single_begin = false;
		char b_char = '\0';
		while (code.get(c)) {
			if (c == '/' and b_char == '/')
				while (code.get(c) and c != '\n');
			else if (c == '*' and b_char == '/')
				while (code.get(c) and c == '/' and b_char == '*')
					b_char = c;
			b_char = c;
			new_code += c;
		};
		return new_code;
	};
	auto parse() {

	};
};

auto main() -> int {
	Parser parser(code);
	cout << parser.clear_comments();
	return 0;
};
