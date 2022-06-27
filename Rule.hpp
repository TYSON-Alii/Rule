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
	enum class word : byte { no, keyw, op, token, number, lit };
	struct Word : str { using str::basic_string; Word() = default; Word(const Word&) = default; Word(const str& s) : str(s) { }; word type = word::no; };
	struct lit_not_in { char beg, end; };
	struct lit_pair { lit_pair(const str& b, const str& e, const lit_not_in& l, bool bs, bool ln_p, bool a_b, bool a_e) : beg(b), end(e), not_in(l), bslash(bs), ln_problem(ln_p), add_beg(a_b), add_end(a_e) { }; str beg, end; lit_not_in not_in; bool bslash, ln_problem, add_beg, add_end; };
	list<str> tokens {"const", "constexpr", "virtual", "static", "inline", "explicit", "friend", "volatile", "register", "short", "long", "signed", "unsigned" };
	list<str> keywords { "fn", "auto", "return", "break", "case", "catch", "class", "concept", "continue", "decltype", "default", "delete", "do", "else", "if", "enum", "export", "extern", "for", "goto", "namespace", "new", "noexcept", "operator", "private", "public", "protected", "requires", "sizeof", "struct", "switch", "template", "throw", "try", "typedef", "typename", "union", "while" };
	list<str> rbracket {"if", "while" };
	list<str> ops {
		"{", "}","[", "]", "(", ")", "<", ">", "=", "+", "-", "/", "*", "%", "&", "|", "^", ".", ":", ",", ";", "?", "@",
		"==", "!=", ">=", "<=", "<<", ">>", "--", "++", "&&", "||", "+=", "-=", "*=", "/=", "%=", "^=", "|=", "&=", "->", "=>", "::", "..",
		"<=>", "<<=", ">>=", "...", "==="
	};
	struct Macro {
		str name, value;
	};
	list<Macro> macros = {
		{{ "and" },{ "&&" }},
		{{ "or" },{ "||" }},
		{{ "bitand" },{ "&" }},
		{{ "bitor" },{ "|" }},
		{{ "not" },{ "!" }},
		{{ "xor" },{ "^" }},
		{{ "and_eq" },{ "&=" }},
		{{ "or_eq" },{ "|=" }},
		{{ "not_eq" },{ "!=" }},
		{{ "xor_eq" },{ "^=" }}
	};
	struct Def {
		str name;
		struct {
			str begin, end;
		} bracket;
		list<str> args;
		list<str> seps;
		list<Word> val;
	};
	list<Def> defs;
	list<lit_pair> lits {
		lit_pair("\"","\"", {'\0','\0'}, true, true, true, true),
		lit_pair("'","'", {'\0','\0'}, true, true, true, true),
		lit_pair("//","\n", {'\0','\0'}, true, false, true, false),
		lit_pair("/*","*/", {'\0','\0'}, false, false, true, true),
		lit_pair("#","\n", {'\0','\0'}, true, false, true, false),
		lit_pair("$def","{", {'\0','\0'}, false, false, true, false),
		lit_pair("$","\n", {'\0','\0'}, true, false, true, false),
		lit_pair("`","`", {'\0','\0'}, false, false, true, true),
		lit_pair("f\"","\"", {'{','}'}, true, true, true, true),
		lit_pair("R\"","\"", {'\0','\0'}, true, true, true, true)
		// maybe later
		// lit_pair("/+","+/", {'\0','\0'}, false)
		// lit_pair("u8\"","\"", {'\0','\0'}, true)
		// lit_pair("L\"","\"", {'\0','\0'}, true)
		// lit_pair("[[","]]", {'\0','\0'}, false)
	};
	Rule() = default;
	Rule(const str& _code) : code(_code) { parse(); };
	str parse() {
		const auto& is_in = [](auto v, auto l) { for (const auto& i : l) if (v == i) return true; return false; };
		const auto& is_macro = [&](const auto& v) { for (const auto& i : macros) if (v == i.name) return true; return false; };
		const auto& is_def = [&](const auto& v) { for (const auto& i : defs) if (v == i.name) return true; return false; };
		const auto& is_digit = [](char c) { return c > char(47) and c < char(58); };
		const auto& add_include = [&](const str& s) { if (!is_in(s, rule_includes)) rule_includes.push_back(s); };
		const auto& go_end = [&](auto& it, auto& val, const auto& ch) {
			int bc = 0, cc = 0, sc = 0, tc = 0;
			while (!(bc == 0 and cc == 0 and sc == 0 and tc == 0 and *it == ch)) {
				if		(*it == "(") bc++;
				else if (*it == ")") bc--;
				else if (*it == "{") cc++;
				else if (*it == "}") cc--;
				else if (*it == "[") sc++;
				else if (*it == "]") sc--;
				else if (*it == "<") tc++;
				else if (*it == ">") tc--;
				val += *it;
				if (*it == "{" or (*it == "}" and *(it + 1) != ";") or *(it + 1) == "}" or *it == ";" or it->type != word::op and (it + 1)->type != word::op or it->type == word::keyw or it->type == word::token) val += ' ';
				it++;
			};
		};
		const auto& split_fn = [&](str s, str delimiter) {
			uint pos_start = 0, pos_end, delim_len = delimiter.length();
			str token;
			list<string> res;
			while ((pos_end = s.find(delimiter, pos_start)) != str::npos) {
				token = s.substr(pos_start, pos_end - pos_start);
				pos_start = pos_end + delim_len;
				res.push_back(token);
			}
			res.push_back(s.substr(pos_start));
			return res;
		};
		split = split_code(code);
		list<Word> temp_split;
		str temp_str;
		for (auto it = split.begin(); it != split.end(); it++) {
			auto& i = *it;
			const auto& it1 = it + 1;
			if (i.starts_with("//") or i.starts_with("/*")) continue;
			else if (i.starts_with("$macro ")) {
				str t(i.begin() + 6, i.end());
				Macro macro;
				uint pos = t.find_first_of(' ');
				if (pos != str::npos) {
					macro.name = str(t.begin(), t.begin() + pos);
					macro.value = str(t.begin() + pos + 1, t.end());
				}
				else {
					macro.name = t;
				}
				macros.push_back(macro);
			}
			else if (i.starts_with("$def ")) {
				str t = str(i.begin() + 5, i.end());
				Def def;
				auto s = split_code(t);
				auto itt = s.begin();
				def.name = *itt;
				itt++;
				def.bracket.begin = *itt;
				if (*itt == "(")
					def.bracket.end = ")";
				else if (*itt == "[")
					def.bracket.end = "]";
				else if (*itt == "<")
					def.bracket.end = ">";
				itt++;
				bool c = false;
				for (; *itt != def.bracket.end; itt++) {
					if (not c)
						def.args.push_back(*itt);
					else
						def.seps.push_back(*itt);
					c = !c;
				}
				it++;
				t.clear();
				go_end(it,t,"}");
				def.val = split_code(t);
				defs.push_back(def);
			}
			else if (i.starts_with("$rep ")) {
				str t = str(i.begin() + 5, i.end()), rep_str;
				uint pos = 0;
				for (const auto& i : t) {
					if (rep_str.empty() and not is_digit(i))
						cerr << "error..";
					else if (is_digit(i))
						rep_str += i;
					else
						break;
					pos++;
				}
				uint rep_count = stoul(rep_str);
				t = str(t.begin() + pos, t.end());
				for (uint i = 0; i <= rep_count; i++) {
					const str& i_str = to_string(i);
					str c = t;
					uint start_pos = 0;
					while((start_pos = c.find("__n__", start_pos)) != str::npos) {
						c.replace(start_pos, 5, i_str);
						start_pos += i_str.length();
					}
					auto s = split_code(c);
					temp_split.insert(temp_split.end(), s.begin(), s.end());
				}
			}
			else if (i.starts_with("$rep[")) {
				str counts = str(i.begin() + 5, i.begin() + i.find(']')), t = str(i.begin() + i.find(']') + 1, i.end());
				const auto& splt_c = split_code(counts);
				uint beg, end;
				if (splt_c.size() == 3 and splt_c[1] == ":") {
					beg = stoul(splt_c.front());
					end = stoul(splt_c.back());
				}
				else
					cerr << "errorke..";
				if (beg < end) {
					for (uint i = beg; i <= end; i++) {
						const str& i_str = to_string(i);
						str c = t;
						uint start_pos = 0;
						while ((start_pos = c.find("__n__", start_pos)) != str::npos) {
							c.replace(start_pos, 5, i_str);
							start_pos += i_str.length();
						}
						auto s = split_code(c);
						temp_split.insert(temp_split.end(), s.begin(), s.end());
					}
				}
				else if (beg > end) {
					for (uint i = beg; i >= end; i--) {
						const str& i_str = to_string(i);
						str c = t;
						uint start_pos = 0;
						while ((start_pos = c.find("__n__", start_pos)) != str::npos) {
							c.replace(start_pos, 5, i_str);
							start_pos += i_str.length();
						}
						auto s = split_code(c);
						temp_split.insert(temp_split.end(), s.begin(), s.end());
						if (end == 0 and i == 0) break;
					}
				}
			}
			else if (is_macro(i)) {
				Macro& macro = *find_if(macros.begin(), macros.end(), [&](const auto& m) { return m.name == i; });
				if (not macro.value.empty()) {
					auto t = split_code(macro.value);
					temp_split.insert(temp_split.end(), t.begin(), t.end());
				}
			}
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
			else if (i.starts_with("$operator ")) {
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
				if (*(it + 1) == "(") {
					auto cit = it+2;
					str c;
					go_end(cit, c, ")");
					if (*(cit + 1) == "{")
						continue;
				}
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
			else if (i == "fn") {
				it++;
				const str& fn_name = *it;
				it++;
				it++;
				str args, body, type = "auto";
				go_end(it, args, ")");
				it++;
				if (*it == "->") it++, type = *it, it++;
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
				if (*it == ";") {
					temp_split.push_back(rule_space + type + ' ' + name + '(' + args + "); }\n");
				}
				else {
					it++;
					go_end(it, body, "}");
					temp_split.push_back(rule_space + type + ' ' + name + '(' + args + ')' + "{ " + body + " }" + "}\n");
					more = true;
				}
			}
			else if (is_def(i)) {
				list<Def> same_n, tsame_n;
				list<str> args;
				for (const auto& d : defs) if (d.name == i) same_n.push_back(d);
				it++;
				for (const auto& s : same_n) if (s.bracket.begin == *it) tsame_n.push_back(s);
				same_n = tsame_n;
				tsame_n.clear();
				const auto& bracket = same_n.back().bracket;
				it++;
				int bc = 0, cc = 0, sc = 0, tc = 0;
				str t;
				uint arg_count = 0;
				list<str> seps;
				while (!(bc == 0 and cc == 0 and sc == 0 and tc == 0 and *it == bracket.end)) {
					if		(*it == "(") bc++;
					else if (*it == ")") bc--;
					else if (*it == "{") cc++;
					else if (*it == "}") cc--;
					else if (*it == "[") sc++;
					else if (*it == "]") sc--;
					else if (*it == "<") tc++;
					else if (*it == ">") tc--;
					if (bc == 0 and cc == 0 and sc == 0 and tc == 0 and *it != "," and *it != ":" and *it != ";") {
						arg_count++;
						t += *it;
						if (*it == "{" or (*it == "}" and *(it + 1) != ";") or *(it + 1) == "}" or *it == ";" or it->type != word::op and (it + 1)->type != word::op or it->type == word::keyw or it->type == word::token) t += ' ';
					}
					else if (*it == "," or *it == ":" or *it == ";") {
						seps.push_back(*it);
						args.push_back(t);
						t.clear();
					}
					it++;
				}
				if (not t.empty()) args.push_back(t);
				for (const auto& s : same_n) if (s.args.size() == arg_count) tsame_n.push_back(s);
				same_n = tsame_n;
				tsame_n.clear();
				if (same_n.empty())
					cerr << "oops.. cannot find correct def.";
				for (const auto& s : same_n) if (s.seps == seps) tsame_n.push_back(s);
				same_n = tsame_n;
				tsame_n.clear();
				if (same_n.empty() or same_n.size() != 1)
					cerr << "oops.. cannot find correct def.";
				else {
					const auto& def = same_n.back();
					struct arg_t : str { using str::basic_string; arg_t() = default; arg_t(const arg_t&) = default; arg_t(const str& s) : str(s) { }; str value; };
					list<arg_t> f_args;
					auto arg_it = args.begin();
					arg_t t_arg;
					for (const auto& i : def.args) {
						t_arg = i;
						t_arg.value = *arg_it;
						arg_it++;
						f_args.push_back(t_arg);
					}
					list<Word> code;
					for (const auto& i : def.val) {
						if (i.type != word::op and is_in(i, f_args)) {
							auto t = i;
							t = find(f_args.begin(), f_args.end(), i)->value;
							temp_split.push_back(t);
						}
						else {
							temp_split.push_back(i);
						}
					}
				}
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
					if (*(it + 1) == "}" or it->type != word::op and (it + 1)->type != word::op or it->type == word::keyw or it->type == word::token)
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
							if (s != l.end) {
								temp_str += *it;
								it++;
							}
							else if (l.bslash and s == l.end and *(it - 1) == '\\') {
								temp_str += *it;
								it++;
							}
							else if (s == l.end) {
								if (not_c == 0)
									break;
								else
									temp_str += *it, it++;
							}
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
		if (!temp_str.empty()) new_splt();
		for (auto it = split.begin(); it != split.end(); it++) {
			const auto& i = *it;
			Word t;
			if (is_digit(i.back())) {
				t.type = word::number;
				if (it != split.begin() and *(it - 1) == "-"s)
					temp_split.pop_back(), t += '-';
				t += i;
				if ((it + 1) != split.end() and *(it + 1) == "."s) {
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
							if (*it == "{" or (*it == "}" and *(it + 1) != ";") or *(it + 1) == "}" or *it == ";" or it->type != word::op and (it + 1)->type != word::op or it->type == word::keyw)
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
