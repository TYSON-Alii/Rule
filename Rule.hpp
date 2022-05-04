#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
auto split_code(const std::string& code) {
	using namespace std;
	using str = string;
	using pmr::vector;
	struct lit_pair { str beg, end; };
	static const auto& is_in = [](auto v, auto l)  { for (const auto& i : l) if (v == i) return true; return false; };
	vector<str> ops {
		"{", "}","[", "]", "(", ")", "<", ">", "=", "+", "-", "/", "*", "%", "&", "|", "^", ".", ":", ",", ";", "?",
		"==", "!=", ">=", "<=", "<<", ">>", "--", "++", "&&", "||", "+=", "-=", "*=", "/=", "%=", "^=", "|=", "&=", "->", "::",
		"<=>", "<<=", ">>="
	};
	sort(ops.begin(), ops.end(), [](const str& first, const str& second){ return first.size() > second.size(); });
	vector<lit_pair> lits { {"\"","\"" }, {"'","'" }, { "//","\n" }, { "/*", "*/" } };
	sort(lits.begin(), lits.end(), [](const auto& first, const auto& second){ return first.beg.size() > second.beg.size(); });
	str temp_str;
	bool con = false;
	vector<str> split;
	static const auto& new_splt = [&]() {
		if (!temp_str.empty()) {
			split.push_back(temp_str);
			temp_str.clear();
		};
	};
	for (auto it = code.begin(); it != code.end(); it++) {
		const auto& i = *it;
		const auto& it_pos = distance(code.begin(), it);
		if (i == ' ' or i == '\t' or i == '\n' or i == '\v' or i == '\f' or i == '\r') {
			new_splt();
			continue;
		};
		for (const auto& l : lits) {
			const auto& len = l.beg.size();
			const auto& end_len = l.end.size();
			if (it_pos + len <= code.size()) {
				const auto& s = str(it, it + len);
				if (s == l.beg) {
					new_splt();
					it += len;
					temp_str += l.beg;
					while (true) {
						const str& s = str(it, it + end_len);
						if (s != l.end) {
							temp_str += *it;
							it++;
						}
						else if (s == l.end and *(it - 1) == '\\') {
							temp_str.pop_back();
							temp_str += *it;
							it++;
						}
						else break;
					};
					it += end_len - 1;
					split.push_back(temp_str + l.end);
					temp_str.clear();
					goto _exit;
				};
			};
		};
		for (const auto& op : ops) {
			const auto& len = op.size();
			if (it_pos + len <= code.size()) {
				const auto& s = str(it, it + len);
				if (s == op) {
					new_splt();
					it += len - 1;
					split.push_back(s);
					goto _exit;
				};
			};
		};
		temp_str += i;
	_exit:;
	};
	return split;
};
