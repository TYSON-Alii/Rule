#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
using namespace std;
class Rule {
private:
	vector<pair<string, string>> types;
	vector<pair<string, string>> macros;
	map<size_t, string> comments;
	stringstream code;
	static inline auto ltrim(std::string s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
			return !std::isspace(ch);
			}));
		return s;
	}
	static inline auto rtrim(std::string s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
			}).base(), s.end());
		return s;
	}
	string strip(const string& str) {
		string t = ltrim(str);
		t = rtrim(t);
		return t;
	};
public:
	inline Rule(const string& _code) { code << _code; };
	auto get_comments() {
		comments.clear();
		string t_string;
		char c;
		char b_char = '\0';
		size_t p = 0u;
		while (code.get(c)) {
			if (c == '*' and b_char == '/') {
				t_string.clear();
				const auto tp = p;
				while (code.get(c)) {
					if (c == '/' and b_char == '*')
						break;
					else
						t_string += b_char = c;
					p++;
				}
				if (!t_string.empty())
					t_string.pop_back();
				comments.insert({ tp,t_string });
			}
			else if (c == '/' and b_char == '/') {
				t_string.clear();
				const auto tp = p;
				while (code.get(c) and c != '\n') t_string += c, p++;
				comments.insert({ tp,t_string });
			};
			b_char = c;
			p++;
		};
		return comments;
	};
	auto clear_comments() {
		string new_code;
		char c;
		bool dont_append = false;
		char b_char = '\0';
		while (code.get(c)) {
			if (c == '*' and b_char == '/') {
				new_code.pop_back();
				while (code.get(c))
					if (c == '/' and b_char == '*')
						break;
					else
						b_char = c;
				//if (code.get(c))
				//	b_char = c;
				dont_append = true;
			}
			else if (c == '/' and b_char == '/') {
				while (code.get(c) and c != '\n');
				new_code.pop_back();
			};
			if (dont_append == true)
				dont_append = false;
			else
				new_code += c;
			b_char = c;
		};
		code << new_code;
		return code.str();
	};
	auto get_macros() {
		string s;
		while (getline(code,s)) {
			stringstream ss;
			ss << s;
			string t;
			ss >> t;
			if (t == "#define") {
			_make:;
				string key, value;
				if (ss >> t) {
					key = t;
					while (ss >> t)
						value += t + ' ';
					if (!value.empty())
						value.pop_back();
					macros.push_back({ key,value });
				}
			}
			else if (t == "#") {
				ss >> t;
				if (t == "define")
					goto _make;
			};
		}
		return macros;
	};
	auto parse() {

	};
};
const auto& code = R"(
#include <aaaaa>
#include "bbbbb"
#define meraba
#define selam cat cut
#define print cout <<
typedef Int int;
/*
// dsfdsfdsf
*/
int main() {
	return 0;
};
)";

auto main() -> int {
	Rule parser(code);
	for (const auto& [k, v] : parser.get_macros())
		cout << k << " : " << v << '\n';
	return 0;
};
