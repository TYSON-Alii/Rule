#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
class Rule {
public:
	using str = string;
	template <typename T>
	using list = pmr::vector<T>;
	struct lit_not_in { char beg, end; };
	struct lit_pair { lit_pair(const str& b, const str& e, const lit_not_in& l) : beg(b), end(e), not_in(l) { }; str beg, end; lit_not_in not_in; };
	list<str> tokens {"const", "constexpr", "virtual", "static", "inline", "explicit", "friend", "volatile", "register", "short", "long", "signed", "unsigned" };
	list<str> keywords { "return", "break", "case", "catch", "class", "concept", "continue", "decltype", "default", "delete", "do", "else", "if", "enum", "export", "extern", "for", "goto", "namespace", "new", "noexcept", "operator", "private", "public", "protected", "requires", "sizeof", "struct", "switch", "template", "throw", "try", "typedef", "typename", "union", "while" };
	list<str> ops {
		"{", "}","[", "]", "(", ")", "<", ">", "=", "+", "-", "/", "*", "%", "&", "|", "^", ".", ":", ",", ";", "?",
		"==", "!=", ">=", "<=", "<<", ">>", "--", "++", "&&", "||", "+=", "-=", "*=", "/=", "%=", "^=", "|=", "&=", "->", "::",
		"<=>", "<<=", ">>=", "..."
	};
	list<lit_pair> lits {
		lit_pair("\"","\"", {'\0','\0'}),
		lit_pair("'","'", {'\0','\0'}),
		lit_pair("//","\n", {'\0','\0'}),
		lit_pair("/*","*/", {'\0','\0'}),
		lit_pair("#","\n", {'\0','\0'}),
		lit_pair("`","`", {'\0','\0'}),
		lit_pair("f\"","\"", {'{','}'})
	};
	Rule(const str& _code) : code(_code) {
		const auto& is_in = [](auto v, auto l)  { for (const auto& i : l) if (v == i) return true; return false; };
		split = split_code(code);
		list<str> temp_split;
		str temp_str;
		for (auto it = split.begin(); it != split.end(); it++) {
			auto& i = *it;
			const auto& it1 = it + 1;
			if (i.starts_with("`")) {
				temp_split.push_back("R(\""s+str(i.begin()+1,i.end()-1)+")\"");
			}
			else if (i.starts_with("f\"")) {
				temp_split.push_back(parse_fliteral(str(i.begin() + 2, i.end() - 1)));
			}
			else if (it1 != split.end()) {
				auto& i1 = *it1;
				if (i == "::") {
					if (it != split.begin() and !(is_in(*(it-1), tokens) or is_in(*(it-1), keywords)))
						temp_split.back() += i + i1;
					else
						temp_split.push_back(i + i1);
					it++;
				}
				else if (i == "operator" and is_in(i1,ops)) {
					temp_split.push_back(i + i1);
					it++;
				}
				else
					temp_split.push_back(i);
			}
			else
				temp_split.push_back(i);
		};
		split = temp_split;
		temp_split.clear();
	};
	list<str> split;
private:
	list<str> split_code(const str& code) {
		const auto& is_in = [](auto v, auto l)  { for (const auto& i : l) if (v == i) return true; return false; };
		sort(ops.begin(), ops.end(), [](const str& first, const str& second){ return first.size() > second.size(); });
		sort(lits.begin(), lits.end(), [](const auto& first, const auto& second){ return first.beg.size() > second.beg.size(); });
		str temp_str;
		list<str> split;
		const auto& new_splt = [&]() {
			if (!temp_str.empty()) {
				split.push_back(temp_str);
				temp_str.clear();
			};
		};
		for (auto it = code.begin(); it != code.end(); it++) {
			const auto& i = *it;
			const auto& it_pos = distance(code.begin(), it);
			if (isspace(i)) {
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
						int not_c = 0;
						while (true) {
							const str& s = str(it, it + end_len);
							if (*it == l.not_in.beg)
								not_c++;
							else if (*it == l.not_in.end)
								not_c--;
							if (s != l.end) {
								temp_str += *it;
								it++;
							}
							else if (l.end.size() == 1 and s == l.end and *(it - 1) == '\\') {
								temp_str.pop_back();
								temp_str += *it;
								it++;
							}
							else
								if (not_c == 0)
									break;
								else
									temp_str += *it, it++;
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
		if (!temp_str.empty()) split.push_back(temp_str);
		return split;
	};
	str parse_fliteral(const str& code, const str& str_t = "std::string", const str& converter = "std::to_string") {
		const auto& is_in = [](auto v, auto l)  { for (const auto& i : l) if (v == i) return true; return false; };
		str s = str_t + "(\"", temp;
		const str& conv_s = "\") + "s + converter + '(';
		const str& add_s  = ") + "s + str_t + "(\"";
		const str& null_s = " + "s + str_t + "(\"\")";
		for (auto it = code.begin(); it != code.end(); it++) {
			const auto& i = *it;
			if (i == '{') {
				it++;
				int bc = 0, cc = 0, sc = 0;
				while (!(bc == 0 and cc == 0 and sc == 0 and *it == '}')) {
					if (*it == '(')
						bc++;
					else if (*it == ')')
						bc--;
					else if (*it == '{')
						cc++;
					else if (*it == '}')
						cc--;
					else if (*it == '[')
						sc++;
					else if (*it == ']')
						sc--;
					temp += *it;
					it++;
				};
				const auto& splt = split_code(temp);
				if (!splt.empty()) {
					s += conv_s;
					temp.clear();
					for (auto it = splt.begin(); it != splt.end(); it++) {
						temp += *it;
						if (it + 1 != splt.end())
							if (*it == "{" or (*it == "}" and *(it + 1) != ";") or *(it + 1) == "}" or *it == ";" or !is_in(*it, ops) and !is_in(*(it + 1), ops) or is_in(*it, keywords))
								temp += ' ';
					};
					s += temp + add_s;
				};
			}
			else {
				s += i;
			}
		};
		s += "\")";
		if (s.starts_with(str_t + "(\"\") + ")) s = str(s.begin() + (str_t + "(\"\") + ").size(), s.end());
		if (s.ends_with(null_s)) s = str(s.begin(), s.end() - null_s.size());
		s = "("s + s + ')';
		return s;
	};
	str code;
};
ostream& operator<<(ostream& os, const Rule& rule) {
	const auto& is_in = [](auto v, auto l)  { for (const auto& i : l) if (v == i) return true; return false; };
	for (auto it = rule.split.begin(); it != rule.split.end(); it++) {
		if (it->starts_with("#"))
			os << '\n';
		os << *it;
		if (it + 1 != rule.split.end())
			if (*it == "{" or (*it == "}" and *(it+1) != ";") or *(it+1) == "}" or *it == ";" or !is_in(*it, rule.ops) and !is_in(*(it + 1), rule.ops) or is_in(*it, rule.keywords) or is_in(*it, rule.tokens))
				os << ' ';
	};
	return os;
};
