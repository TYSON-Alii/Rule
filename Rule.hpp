#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
using namespace std;
class Rule {
public:
	using str = string;
	using uint = unsigned int;
	template <typename T>
	using list = pmr::vector<T>;
	struct lit_not_in { char beg, end; };
	struct lit_pair { lit_pair(const str& b, const str& e, const lit_not_in& l, const bool& bs, const bool& ln_p) : beg(b), end(e), not_in(l), bslash(bs), ln_problem(ln_p) { }; str beg, end; lit_not_in not_in; bool bslash, ln_problem; };
	list<str> tokens {"const", "constexpr", "virtual", "static", "inline", "explicit", "friend", "volatile", "register", "short", "long", "signed", "unsigned" };
	list<str> keywords { "return", "break", "case", "catch", "class", "concept", "continue", "decltype", "default", "delete", "do", "else", "if", "enum", "export", "extern", "for", "goto", "namespace", "new", "noexcept", "operator", "private", "public", "protected", "requires", "sizeof", "struct", "switch", "template", "throw", "try", "typedef", "typename", "union", "while" };
	list<str> rbracket {"if", "while" };
	list<str> ops {
		"{", "}","[", "]", "(", ")", "<", ">", "=", "+", "-", "/", "*", "%", "&", "|", "^", ".", ":", ",", ";", "?",
		"==", "!=", ">=", "<=", "<<", ">>", "--", "++", "&&", "||", "+=", "-=", "*=", "/=", "%=", "^=", "|=", "&=", "->", "::", "..",
		"<=>", "<<=", ">>=", "...", "==="
	};
	list<lit_pair> lits {
		lit_pair("\"","\"", {'\0','\0'}, true, true),
		lit_pair("'","'", {'\0','\0'}, true, true),
		lit_pair("//","\n", {'\0','\0'}, true, false),
		lit_pair("/*","*/", {'\0','\0'}, false, false),
		lit_pair("#","\n", {'\0','\0'}, true, false),
		lit_pair("`","`", {'\0','\0'}, false, false),
		lit_pair("f\"","\"", {'{','}'}, true, true)
		// maybe later
		// lit_pair("/+","+/", {'\0','\0'}, false)
		// lit_pair("u8\"","\"", {'\0','\0'}, true)
		// lit_pair("L\"","\"", {'\0','\0'}, true)
		// lit_pair("[[","]]", {'\0','\0'}, false)
	};
	//struct sstr : str { sstr() = default; sstr(const sstr&) = default; sstr(const str& s) : str(s) { }; str after; };
	Rule() = default;
	Rule(const str& _code) : code(_code) { parse(); };
	str parse() {
		const auto& is_in = [](auto v, auto l) { for (const auto& i : l) if (v == i) return true; return false; };
		const auto& is_digit = [](char c) { return c > char(47) and c < char(58); };
		const auto& add_include = [&](const str& s) { if (!is_in(s, rule_includes)) rule_includes.push_back(s); };
		const auto& go_end = [&](auto& it, auto& val, const auto& ch1, const auto& ch2) {
			int bc = 0, cc = 0, sc = 0;
			while (!(bc == 0 and cc == 0 and sc == 0 and (*it == ch1 or *it == ch2))) {
				if		(*it == "(") bc++;
				else if (*it == ")") bc--;
				else if (*it == "{") cc++;
				else if (*it == "}") cc--;
				else if (*it == "[") sc++;
				else if (*it == "]") sc--;
				val += *it;
				if (*it == "{" or (*it == "}" and *(it + 1) != ";") or *(it + 1) == "}" or *it == ";" or !is_in(*it, ops) and !is_in(*(it + 1), ops)) val += ' ';
				it++;
			};
		};
		split = split_code(code);
		list<str> temp_split;
		str temp_str;
		for (auto it = split.begin(); it != split.end(); it++) {
			auto& i = *it;
			const auto& it1 = it + 1;
			if (i.starts_with("//") or i.starts_with("/*")) continue;
			else if (i.starts_with("#redefine ")) {
				const str& s = str(i.begin() + 10, i.end()-1);
				const auto& f = std::find_if(s.begin(), s.end(), [](auto ch) { return std::isspace(ch); });
				const str& m = str(s.begin(), f);
				const str& val = str(f, s.end());
				temp_split.push_back("#ifdef "s + m + '\n');
				temp_split.push_back("#undef "s + m + '\n');
				temp_split.push_back("#endif"s + '\n');
				temp_split.push_back("#define "s + s + '\n');
			}
			else if (i.starts_with("#operator ")) {
				const auto& s = str(i.begin() + 10, i.end() - 1);
				rule_user_ops.push_back(rule_user_op + s + "();");
				user_ops.push_back(s);
			}
			else if (it1 != split.end()) {
				auto& i1 = *it1;
				if (i == "::") {
					if (it != split.begin() and !(is_in(*(it - 1), tokens) or is_in(*(it - 1), keywords)))
						temp_split.back() += i + i1;
					else
						temp_split.push_back(i + i1);
					it++;
				}
				else if (i == ".") {
					if (it != split.begin() and !(is_in(*(it - 1), tokens) or is_in(*(it - 1), keywords)))
						temp_split.back() += i + i1;
					else
						temp_split.push_back(i + i1);
					it++;
				}
				else if (i == "operator" and is_in(i1, ops)) {
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
		// 2nd iterate
		for (auto it = split.begin(); it != split.end(); it++) {
			const auto& i = *it;
			const auto& it1 = it + 1;
			if (i.starts_with("`")) {
				add_include("string");
				rule_to_string_funcs.add = true;
				temp_split.push_back("R\"__cxx_rule("s + str(i.begin() + 1, i.end() - 1) + ")__cxx_rule\"");
			}
			else if (i.starts_with("f\"")) {
				add_include("string");
				rule_to_string_funcs.add = true;
				temp_split.push_back(parse_fliteral(str(i.begin() + 2, i.end() - 1)));
			}
			else if (is_in(i, rbracket)) {
				temp_split.push_back(i);
				temp_split.push_back("(");
				it++;
				int bc = 0, cc = 0, sc = 0;
				while (!(bc == 0 and cc == 0 and sc == 0 and *it == "{")) {
					if (*it == "(")
						bc++;
					else if (*it == ")")
						bc--;
					else if (*it == "{")
						cc++;
					else if (*it == "}")
						cc--;
					else if (*it == "[")
						sc++;
					else if (*it == "]")
						sc--;
					temp_split.push_back(*it);
					it++;
				};
				temp_split.push_back(")");
				temp_split.push_back("{");
			}
			else if (i == "operator" and is_in(*it1, user_ops)) {
				str type, args, body;
				temp_split.pop_back();
				it--;
				const auto t = it;
				if (it != split.begin()) {
					it--;
					while (it != split.begin() and !temp_split.empty() and is_in(*it, tokens)) temp_split.pop_back(), it--;
					it++;
					while (it != t) type += *it + ' ', it++;
				};
				type += *t;
				it+=2;
				const str& name = "__operator_"s + *it;
				it+=2;
				go_end(it, args, ",", ")");
				it++;
				it++;
				go_end(it, body, "}", "");
				temp_split.push_back(rule_space + type + ' ' + name + '(' + args + ')' + "{ " + body + " }" + "}\n");
			}
			else if (is_in(i, user_ops)) {
				str op, var, v;
				list<str> vl;
				uint op_c = 1;
				op = i;
				it++;
				var = *it;
				vl.push_back("__cxx_rule::__operator_"s + op);
				while (is_in(var, user_ops)) {
					op_c++;
					op = var;
					it++;
					var = *it;
					vl.push_back("__cxx_rule::__operator_"s + op);
				}
				for (const str& s : vl) v += s + '(';
				v += var;
				for (uint i = 0; i < op_c; i++) v += ')';
				temp_split.push_back(v);
			}
			else if (i == "..") {
				temp_split.pop_back();
				if ((((it - 1)->front() == '-' and all_of((it - 1)->begin() + 1, (it - 1)->end(), is_digit)) or all_of((it - 1)->begin(), (it - 1)->end(), is_digit)) and
					(((it + 1)->front() == '-' and all_of((it + 1)->begin() + 1, (it + 1)->end(), is_digit)) or all_of((it + 1)->begin(), (it + 1)->end(), is_digit))) {
					const auto& beg = stoi(*(it - 1));
					const auto& end = stoi(*(it + 1));
					if (beg < end) {
						const auto& e = end - 1;
						for (int i = beg; i < e; i++) temp_split.push_back(to_string(i)), temp_split.push_back(","s);
						temp_split.push_back(to_string(e));
					}
					else if (beg > end) {
						const auto& e = end + 1;
						for (int i = beg; i > e; i--) temp_split.push_back(to_string(i)), temp_split.push_back(","s);
						temp_split.push_back(to_string(e));
					}
					else
						temp_split.push_back(to_string(beg));
				}
				else {
					rule_dotdot_op.add = true;
					add_include("vector");
					temp_split.push_back("__cxx_rule::dotdot_op("s + *(it - 1) + ',' + *(it + 1) + ')');
				};
				it++;
			}
			else
				temp_split.push_back(i);
		}
		split = temp_split;
		temp_split.clear();
		auto& ac = afterCode;
		ac.clear();
		for (const auto& i : rule_includes)
			ac += "#include <"s + i + ">\n";
		if (rule_to_string_funcs.add)
			ac += rule_to_string_funcs.code;
		ac += rule_space + '\n';
		if (rule_dotdot_op.add)
			ac += rule_dotdot_op.code;
		for (const auto& op : rule_user_ops)
			ac += op + '\n';
		ac += "};\n\n";
		uint tab = 0;
		for (auto it = split.begin(); it != split.end(); it++) {
			ac += *it;
			if (it + 1 != split.end())
				if (*it == ";" or *it == "{" or *it == "}") {
					if (*it == "{") tab++;
					else if (*it == "}") {
						if (*(ac.end() - 2) == '\t')
							ac.erase(ac.end() - 2);
						tab--;
					};
					ac += '\n';
					for (uint i = 0; i < tab; i++)
						ac += '\t';
				}
				else if (!ac.ends_with("\n"))
					if (*(it + 1) == "}" or !is_in(*it, ops) and !is_in(*(it + 1), ops) or is_in(*it, keywords) or is_in(*it, tokens))
						ac += ' ';
		};
		return afterCode;
	};
	list<str> split;
	str afterCode;
private:
	list<str> user_ops, rule_user_ops, rule_includes;
	const str& rule_space = "namespace __cxx_rule { ";
	struct fn_t {
		bool add = false;
		const str code;
	};
	fn_t rule_dotdot_op { false, R"(auto __dotdot_op(auto beg, auto end) {
	std::vector<decltype(beg)> list;
	if (beg < end) for (auto i = beg; i < end; i++) list.push_back(i);
	else if (beg > end) for (auto i = beg; i > end; i--) list.push_back(i);
	else list.push_back(beg);
	return list;
};
)" };
	fn_t rule_to_string_funcs = { false, R"(namespace std {
	inline string to_string(string s) { return s; };
	inline string to_string(string* s) { return *s; };
};
)" };
	const str& rule_user_op = "void __operator_";
	list<str> split_code(const str& code) {
		const auto& is_in = [](auto v, auto l)  { for (const auto& i : l) if (v == i) return true; return false; };
		const auto& is_digit = [](char c)  { return c > char(47) and c < char(58); };
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
			}
			else if (i == '-') {
				if (is_digit(*(it + 1))) {
					temp_str.clear();
					temp_str += '-';
					it++;
					while (is_digit(*it)) temp_str += *it, it++;
					new_splt();
					it--;
					continue;
				};
			}
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
							if (*it == '\n' and l.ln_problem and *(it - 1) != '\\')
								throw "errorke";
							else if (s != l.end) {
								temp_str += *it;
								it++;
							}
							else if (l.bslash and s == l.end and *(it - 1) == '\\') {
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
			temp_str += *it;
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
	for (auto it = rule.split.begin(); it != rule.split.end(); it++) {
		os << *it << '\n';
	};
	return os;
};
