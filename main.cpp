#include <iostream>
#include <vector>
#include <string>
#include <sstream>
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
		bool dont_append = false;
		char b_char = '\0';
		while (code.get(c)) {
			if (c == '/' and b_char == '/') {
				while (code.get(c) and c != '\n');
				new_code.pop_back();
			}
			else if (c == '*' and b_char == '/') {
				new_code.pop_back();
				while (code.get(c) and c != '/' and b_char != '*')
					b_char = c;
				dont_append = true;
			};
			b_char = c;
			if (dont_append == true)
				dont_append = false;
			else
				new_code += c;
		};
		return new_code;
	};
	auto parse() {

	};
};
const auto& code = R"(
#include <aaaaa>
#include "bbbbb"
#define meraba
typedef Int int;

/*
dfds
fsd
fdsf
*/

int main() { // dsdsadsad
	return 0;
};
)";
auto main() -> int {
	Parser parser(code);
	cout << parser.clear_comments();
	return 0;
};
