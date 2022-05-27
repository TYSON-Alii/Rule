#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
using namespace std;
class Rule {
public:
	using str = string;
	using uint = unsigned int;
	using byte = unsigned char;
	template <typename T>
	using list = pmr::vector<T>;
	struct lit_not_in { char beg, end; };
	struct lit_pair { lit_pair(const str& b, const str& e, const lit_not_in& l, bool bs, bool ln_p, bool a_b, bool a_e) : beg(b), end(e), not_in(l), bslash(bs), ln_problem(ln_p), add_beg(a_b), add_end(a_e) { }; str beg, end; lit_not_in not_in; bool bslash, ln_problem, add_beg, add_end; };
	list<str> tokens {"const", "constexpr", "virtual", "static", "inline", "explicit", "friend", "volatile", "register", "short", "long", "signed", "unsigned" };
	list<str> keywords { "fn", "auto", "return", "break", "case", "catch", "class", "concept", "continue", "decltype", "default", "delete", "do", "else", "if", "enum", "export", "extern", "for", "goto", "namespace", "new", "noexcept", "operator", "private", "public", "protected", "requires", "sizeof", "struct", "switch", "template", "throw", "try", "typedef", "typename", "union", "while" };
	list<str> rbracket {"if", "while" };
	list<str> ops {
		"{", "}","[", "]", "(", ")", "<", ">", "=", "+", "-", "/", "*", "%", "&", "|", "^", ".", ":", ",", ";", "?",
		"==", "!=", ">=", "<=", "<<", ">>", "--", "++", "&&", "||", "+=", "-=", "*=", "/=", "%=", "^=", "|=", "&=", "->", "::", "..",
		"<=>", "<<=", ">>=", "...", "==="
	};
	list<lit_pair> lits {
		lit_pair("\"","\"", {'\0','\0'}, true, true, true, true),
		lit_pair("'","'", {'\0','\0'}, true, true, true, true),
		lit_pair("//","\n", {'\0','\0'}, true, false, true, false),
		lit_pair("/*","*/", {'\0','\0'}, false, false, true, true),
		lit_pair("#","\n", {'\0','\0'}, true, false, true, false),
		lit_pair("`","`", {'\0','\0'}, false, false, true, true),
		lit_pair("f\"","\"", {'{','}'}, true, true, true, true)
		// maybe later
		// lit_pair("/+","+/", {'\0','\0'}, false)
		// lit_pair("u8\"","\"", {'\0','\0'}, true)
		// lit_pair("L\"","\"", {'\0','\0'}, true)
		// lit_pair("[[","]]", {'\0','\0'}, false)
	};
	enum class word : byte { no, keyw, op, token, number, lit };
	struct Word : str { using str::basic_string; Word() = default; Word(const Word&) = default; Word(const str& s) : str(s) { }; word type = word::no; };
	Rule() = default;
	Rule(const str& _code) : code(_code) { parse(); };
	str parse() {
		const auto& is_in = [](auto v, auto l) { for (const auto& i : l) if (v == i) return true; return false; };
		const auto& is_digit = [](char c) { return c > char(47) and c < char(58); };
		const auto& add_include = [&](const str& s) { if (!is_in(s, rule_includes)) rule_includes.push_back(s); };
		const auto& go_end = [&](auto& it, auto& val, const auto& ch) {
			int bc = 0, cc = 0, sc = 0;
			while (!(bc == 0 and cc == 0 and sc == 0 and *it == ch)) {
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
		list<Word> temp_split;
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
				temp_split.push_back("#ifdef "s + m);
				temp_split.push_back("#undef "s + m);
				temp_split.push_back("#endif"s);
				temp_split.push_back("#define "s + s);
			}
			else if (i.starts_with("#operator ")) {
				const auto& s = str(i.begin() + 10, i.end());
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
			if (i.back() == '`') {
				add_include("string");
				rule_to_string_funcs.add = true;
				temp_split.push_back("R\"__cxx_rule("s + str(i.begin() + 1, i.end() - 1) + ")__cxx_rule\"");
			}
			else if (i.starts_with("f\"")) {
				add_include("string");
				rule_to_string_funcs.add = true;
				temp_split.push_back(parse_fliteral(str(i.begin() + 2, i.end() - 1)));
				more = true;
			}
			else if (is_in(i, rbracket)) {
				temp_split.push_back(i);
				Word t;
				t = "(";
				t.type = word::op;
				temp_split.push_back(t);
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
				t.type = word::op;
				t = ")";
				temp_split.push_back(")");
				t = "{";
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
				go_end(it, args, ")");
				it++;
				it++;
				go_end(it, body, "}");
				temp_split.push_back(rule_space + type + ' ' + name + '(' + args + ')' + "{ " + body + " }" + "}\n");
				more = true;
			}
			else if (i == "fn") {
				it++;
				const str& fn_name = *it;
				it++;
				it++;
				str args, body, type = "auto";
				go_end(it, args, ")");
				it++;
				if (*it == "->") it++, type = *it;
				it++;
				go_end(it, body, "}");
				body += '}';
				temp_split.push_back("struct { "s + type + " operator()(" + args + ')' + '{' + body + '}' + fn_name + ';');
				more = true;
			}
			else if (i == "..") {
				temp_split.pop_back();
				if ((it-1)->type == word::number and (it+1)->type == word::number) {
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
					temp_split.push_back("__cxx_rule::__dotdot_op("s + *(it - 1) + ',' + *(it + 1) + ')');
				};
				it++;
			}
			else if (is_in(i, user_ops)) {
				str op, var;
				Word v;
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
				v.type = word::no;
				for (uint i = 0; i < op_c; i++) v += ')';
				temp_split.push_back(v);
			}
			else
				temp_split.push_back(i);
		}
		split = temp_split;
		temp_split.clear();
		auto& ac = afterCode;
		ac.clear();
		if (!more) {
			for (const auto& i : rule_includes)
				ac += "#include <"s + i + ">\n";
			if (rule_to_string_funcs.add)
				ac += rule_to_string_funcs.code;
			ac += rule_space + '\n';
			if (rule_dotdot_op.add)
				ac += rule_dotdot_op.code;
			ac += "};\n\n";
		}
		uint tab = 0;
		for (auto it = split.begin(); it != split.end(); it++) {
			ac += *it;
			if (it->starts_with("#"))
				ac += '\n';
			else if (it + 1 != split.end())
				if (it->ends_with(";") or it->ends_with("{") or it->ends_with("}") or it->ends_with(";")) {
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
		}
		if (more) {
			more = false;
			code = afterCode;
			parse();
		}
		return afterCode;
	};
	bool more = false;
	list<Word> split;
	str code, afterCode;
private:
	list<str> user_ops, rule_includes;
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
	list<Word> split_code(const str& code) {
		const auto& is_in = [](auto v, auto l)  { for (const auto& i : l) if (v == i) return true; return false; };
		const auto& is_digit = [](char c)  { return c > char(47) and c < char(58); };
		sort(ops.begin(), ops.end(), [](const str& first, const str& second){ return first.size() > second.size(); });
		sort(lits.begin(), lits.end(), [](const auto& first, const auto& second){ return first.beg.size() > second.beg.size(); });
		Word temp_str;
		list<Word> split, temp_split;
		const auto& is_number = [&](const str& s) {
			if (s.empty() || ((!is_digit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;
			for (const auto& i : s) if (i != '.' and !is_digit(i)) return false;
			return true;
		};
		const auto& new_splt = [&]() {
			if (!temp_str.empty()) {
				if (is_in(temp_str, ops))
					temp_str.type = word::op;
				else if (is_in(temp_str, tokens))
					temp_str.type = word::token;
				else if (is_number(temp_str))
					temp_str.type = word::number;
				else if (is_in(temp_str, keywords))
					temp_str.type = word::keyw;
				else
					temp_str.type = word::no;
				split.push_back(temp_str);
				temp_str = Word();
			};
		};
		for (auto it = code.begin(); it != code.end(); it++) {
			const auto& i = *it;
			const auto& it_pos = distance(code.begin(), it);
			if (isspace(i)) {
				new_splt();
				continue;
			}
			for (const auto& l : lits) {
				const auto& len = l.beg.size();
				const auto& end_len = l.end.size();
				if (it_pos + len <= code.size()) {
					const auto& s = str(it, it + len);
					if (s == l.beg) {
						new_splt();
						it += len;
						if (l.add_beg)
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
						temp_str.type = word::lit;
						if (l.add_end)
							temp_str += l.end;
						split.push_back(temp_str);
						temp_str = Word();
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
						temp_str = s;
						new_splt();
						goto _exit;
					};
				};
			};
			temp_str += *it;
		_exit:;
		};
		for (auto it = split.begin(); it != split.end(); it++) {
			const auto& i = *it;
			Word t;
			if (is_digit(i.back())) {
				t.type = word::number;
				if (*(it - 1) == "-"s)
					temp_split.pop_back(), t += '-';
				t += i;
				if (*(it + 1) == "."s) {
					t += '.';
					it++;
					t += *(it + 1);
					it++;
				}
			}
			else
				t = i;
			temp_split.push_back(t);
		}
		split = temp_split;
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
};
ostream& operator<<(ostream& os, const Rule& rule) {
	for (auto it = rule.split.begin(); it != rule.split.end(); it++) {
		os << *it << '\n';
	};
	return os;
};
