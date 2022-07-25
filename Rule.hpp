#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include "tinyexpr.h"
using namespace std;
class Rule {
public:
	using str = string;
	using uint = unsigned int;
	using byte = unsigned char;
	template <typename T>
	using list = pmr::vector<T>;
	enum class word : byte { no, keyw, op, token, number, lit };
	struct Word : str {
		using str::basic_string;
		Word() = default;
		Word(const Word&) = default;
		Word(const str& s) : str(s) { };
		Word(const str& s, word t) : str(s), type(t) { };
		word type = word::no;
	};
	struct lit_not_in { char beg, end; };
	struct lit_pair {
		lit_pair(const str& b, const str& e, const lit_not_in& l, bool bs, bool ln_p, bool a_b, bool a_e, bool cbdf = false) : beg(b), end(e), not_in(l), bslash(bs), ln_problem(ln_p), add_beg(a_b), add_end(a_e), can_be_def(cbdf) { };
		str beg, end;
		lit_not_in not_in;
		bool bslash, ln_problem, add_beg, add_end, can_be_def = false;
	};
	list<str> tokens {"const", "constexpr", "virtual", "static", "inline", "explicit", "friend", "volatile", "register", "short", "long", "signed", "unsigned" };
	list<str> keywords { "fn", "auto", "return", "break", "case", "catch", "class", "concept", "continue", "decltype", "default", "delete", "do", "else", "if", "enum", "export", "extern", "for", "goto", "namespace", "new", "noexcept", "operator", "private", "public", "protected", "requires", "sizeof", "struct", "switch", "template", "throw", "try", "typedef", "typename", "union", "while" };
	list<str> rbracket {"if", "while" };
	list<str> ops {
		"{", "}","[", "]", "(", ")", "<", ">", "=", "+", "-", "/", "*", "%", "&", "|", "^", ".", ":", ",", ";", "?", "@",
		"==", "!=", ">=", "<=", "<<", ">>", "--", "++", "&&", "||", "+=", "-=", "*=", "/=", "%=", "^=", "|=", "&=", "->", "=>", "::", "..",
		"<=>", "<<=", ">>=", "...", "==="
	};
	struct OP : str {
		using str::basic_string;
		OP() = default;
		OP(const OP&) = default;
		OP(const str& s) : str(s) { };
		bool unary = true; // op[args], op() args, op<>... ekle
	};
	list<OP> user_ops;
	struct Macro {
		str name;
		list<Word> value;
	};
	list<Macro> macros;
	struct Def {
		str name;
		struct {
			str begin, end;
		} bracket;
		list<str> args;
		list<str> seps;
		list<Word> val;
		bool variadic = false;
	};
	list<Def> defs;
	list<lit_pair> lits {
		lit_pair("\"","\"", {'\0','\0'}, 1, 1, 1, 1, 1),
		lit_pair("'","'", {'\0','\0'}, 1, 1, 1, 1, 1),
		lit_pair("//","\n", {'\0','\0'}, 1, 0, 1, 0, 0),
		lit_pair("/*","*/", {'\0','\0'}, 0, 0, 1, 1, 0),
		lit_pair("#","\n", {'\0','\0'}, 1, 0, 1, 0, 0),
		lit_pair("$def","{", {'\0','\0'}, 0, 0, 1, 0, 0),
		lit_pair("$if"," ", {'\0','\0'}, 0, 0, 1, 0, 0),
		lit_pair("$","\n", {'\0','\0'}, 1, 0, 1, 0, 0),
		lit_pair("`","`", {'\0','\0'}, 0, 0, 1, 1, 1),
		lit_pair("f\"","\"", {'{','}'}, 1, 1, 1, 1, 1),
		lit_pair("R\"","\"", {'\0','\0'}, 1, 1, 1, 1, 1),
		lit_pair("[[","]]", {'\0','\0'}, 0, 0, 1, 1, 0) // convert initializer list to std::vector
		// maybe later
		// lit_pair("/+","+/", {'\0','\0'}, false)
		// lit_pair("u8\"","\"", {'\0','\0'}, true)
		// lit_pair("L\"","\"", {'\0','\0'}, true)
	};
	Rule() = default;
	Rule(const str& _code) : code(_code) { parse(); };
	Rule(const str& _code, Rule* parent) : code(_code), is_child(true) {
		macros = parent->macros;
		defs = parent->defs;
		parse();
		parent->macros = macros;
		parent->defs = defs;
	};
	str parse() {
		const auto& is_in = [](auto v, auto l) { for (const auto& i : l) if (v == i) return true; return false; };
		const auto& is_macro = [&](const auto& v) { for (const auto& i : macros) if (v == i.name) return true; return false; };
		const auto& is_def = [&](const auto& v) { for (const auto& i : defs) if (v == i.name) return true; return false; };
		const auto& is_digit = [](char c) { return c > char(47) and c < char(58); };
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
			else if (it1 != split.end()) {
				auto& i1 = *it1;
				if (i == "::" or i == "->" or i == ".") {
					if (it != split.begin() and (it - 1)->type == word::no)// not ((it - 1)->type == word::token or (it - 1)->type == word::keyw or (it - 1)->type == word::op))
						temp_split.back() += i + i1;
					else
						temp_split.push_back(i + i1);
					it++;
				}
				else if (i == "operator" and i1.type == word::op) {
					temp_split.push_back(i + i1);
					it++;
				}
				else
					temp_split.push_back(i);
			}
			else
				temp_split.push_back(i);
		}
		split = temp_split;
		temp_split.clear();
		// 2nd iterate
		for (auto it = split.begin(); it != split.end(); it++) {
			const auto& i = *it;
			const auto& it1 = it + 1;
			if (i.starts_with("#redefine ")) {
				const str& s = str(i.begin() + 10, i.end() - 1);
				const auto& f = std::find_if(s.begin(), s.end(), [](auto ch) { return std::isspace(ch); });
				const str& m = str(s.begin(), f);
				const str& val = Rule(str(f, s.end()), this).afterCode;
				temp_split.push_back("#ifdef "s + m);
				temp_split.push_back("#undef "s + m);
				temp_split.push_back("#endif"s);
				temp_split.push_back("#define "s + m + ' ' + val);
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
					auto s = Rule(c, this).split;
					temp_split.insert(temp_split.end(), s.begin(), s.end());
				}
			}
			else if (i.starts_with("$rep[")) {
				str counts = str(i.begin() + 5, i.begin() + i.find(']')), t = str(i.begin() + i.find(']') + 1, i.end());
				const str& beg_str = Rule(str(counts.begin(), counts.begin() + counts.find_first_of(':')), this).afterCode;
				const str& end_str = Rule(str(counts.begin() + counts.find_first_of(':') + 1, counts.end()), this).afterCode;
				uint beg = (uint)te_interp(beg_str.c_str(), 0), end = (uint)te_interp(end_str.c_str(), 0);
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
						auto s = Rule(c, this).split;
						temp_split.insert(temp_split.end(), s.begin(), s.end());
						if (end == 0 and i == 0) break;
					}
				}
			}
			else if (i.starts_with("$macro ")) {
				str t(i.begin() + 7, i.end());
				Macro macro;
				uint pos = t.find_first_of(' ');
				if (pos != str::npos) {
					macro.name = str(t.begin(), t.begin() + pos);
					macro.value = Rule(str(t.begin() + pos + 1, t.end()), this).split;
				}
				else {
					macro.name = t;
				}
				auto f = find_if(macros.begin(), macros.end(), [&](const auto& i) { return i.name == macro.name; });
				if (f == macros.end())
					macros.push_back(macro);
				else
					*f = macro;
			}
			else if (i.starts_with("$operator ")) {
				user_ops.push_back(OP { str(i.begin() + 9, i.end()), true });
			}
			else if (i.starts_with("$def ")) {
				str t = str(i.begin() + 5, i.end());
				Def def;
				auto s = split_code(t);
				auto itt = s.begin();
				def.name = *itt;
				itt++;
				if (itt->type == word::lit) {
					const auto lit = *find_if(lits.begin(), lits.end(), [&](auto&& i) { return itt->starts_with(i.beg) and itt->ends_with(i.end); });
					def.bracket.begin = lit.beg;
					def.bracket.end = lit.end;
					def.args.push_back(str(itt->begin() + lit.beg.length(), itt->end() - lit.end.length()));
				}
				else {
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
					if (def.args.back() == "...") {
						def.variadic = true;
						def.args.pop_back();
					}
				}
				it++;
				t.clear();
				go_end(it,t,"}");
				def.val = Rule(t, this).split;
				defs.push_back(def);
			}
			else
				temp_split.push_back(i);
		}
		split = temp_split;
		temp_split.clear();
		// 3th iterate
		for (auto it = split.begin(); it != split.end(); it++) {
			const auto& i = *it;
			const auto& it1 = it + 1;
			if (is_macro(i)) {
				Macro& macro = *find_if(macros.begin(), macros.end(), [&](const auto& m) { return m.name == i; });
				if (not macro.value.empty()) {
					temp_split.insert(temp_split.end(), macro.value.begin(), macro.value.end());
				}
			}
			else if (is_def(i)) {
				list<Def> same_n, tsame_n;
				list<str> args;
				for (const auto& d : defs) if (d.name == i) same_n.push_back(d);
				it++;
				const bool is_lit = it->type == word::lit;
				if (is_lit) {
					const auto lit = *find_if(lits.begin(), lits.end(), [&](auto&& i) { return it->starts_with(i.beg) and it->ends_with(i.end); });
					const auto r = Rule(str(it->begin() + lit.beg.size(), it->end() - lit.end.size())).split;
					for (const auto& s : same_n) if (it->starts_with(s.bracket.begin) and it->ends_with(s.bracket.end)) tsame_n.push_back(s);
					same_n = tsame_n;
					tsame_n.clear();
					if (same_n.empty() or same_n.size() != 1)
						cerr << "oops.. cannot find correct def.\n";
					else {
						const auto& def = same_n.back();
						for (auto it = def.val.begin(); it != def.val.end(); it++) {
							const auto& i = *it;
							if (i == "__def__") {
								temp_split.push_back(def.name);
							}
							if (i == "__arg__" or i == def.args.back()) {
								temp_split.insert(temp_split.end(), r.begin(), r.end());
							}
							else if (i == "__bracket_beg__") {
								temp_split.push_back(def.bracket.begin);
							}
							else if (i == "__bracket_end__") {
								temp_split.push_back(def.bracket.end);
							}
						}
					}
				}
				else {
					for (const auto& s : same_n) if (s.bracket.begin == *it) tsame_n.push_back(s);
					same_n = tsame_n;
					tsame_n.clear();
					const auto& bracket = same_n.back().bracket;
					it++;
					int bc = 0, cc = 0, sc = 0, tc = 0;
					str t;
					list<str> seps;
					while (!(bc == 0 and cc == 0 and sc == 0 and tc == 0 and *it == bracket.end)) {
						if (*it == "(") bc++;
						else if (*it == ")") bc--;
						else if (*it == "{") cc++;
						else if (*it == "}") cc--;
						else if (*it == "[") sc++;
						else if (*it == "]") sc--;
						else if (*it == "<") tc++;
						else if (*it == ">") tc--;
						if (bc == 0 and cc == 0 and sc == 0 and tc == 0 and *it != "," and *it != ":" and *it != ";") {
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
					for (const auto& s : same_n) if (s.args.size() == args.size() or (s.variadic and s.args.size() <= args.size())) tsame_n.push_back(s);
					same_n = tsame_n;
					tsame_n.clear();
					for (const auto& s : same_n)
						if (s.seps == seps or (s.variadic and s.seps == list<str>(seps.begin(), seps.begin() + s.seps.size()) and (s.seps.empty() or s.seps.size() == seps.size() or all_of(seps.begin() + s.seps.size(), seps.end(), [&](const auto& i) { return i == s.seps.back(); }))))
							tsame_n.push_back(s);
					same_n = tsame_n;
					tsame_n.clear();
					if (same_n.size() > 1) {
						for (const auto& i : same_n)
							if (not i.variadic)
								tsame_n.push_back(i);
						if (tsame_n.empty()) {
							auto first = same_n.begin(), last = same_n.end(), lowest = first;
							while (++first != last)
								if (first->seps.size() < lowest->seps.size())
									lowest = first;
							tsame_n.push_back(*lowest);
						}
						same_n = tsame_n;
						tsame_n.clear();
					}
					if (same_n.empty() or same_n.size() != 1)
						cerr << "oops.. cannot find correct def.\n";
					else {
						const auto& def = same_n.back();
						struct arg_t : str { using str::basic_string; arg_t() = default; arg_t(const arg_t&) = default; arg_t(const str& s) : str(s) { }; str value; };
						list<arg_t> f_args;
						auto arg_it = args.begin();
						auto va_args = list<str>(args.begin() + def.args.size(), args.end());
						arg_t t_arg;
						for (const auto& i : def.args) {
							t_arg = i;
							t_arg.value = *arg_it;
							arg_it++;
							f_args.push_back(t_arg);
						}
						for (auto it = def.val.begin(); it != def.val.end(); it++) {
							const auto& i = *it;
							if (i.type != word::op and is_in(i, f_args)) {
								auto t = i;
								t = Rule(find(f_args.begin(), f_args.end(), i)->value, this).afterCode;
								temp_split.push_back(t);
							}
							else if (def.variadic and not va_args.empty() and i == "..." and *(it - 1) == "[") {
								temp_split.pop_back();
								it++;
								str sep;
								if (*it == "]")
									sep = ",";
								else {
									go_end(it, sep, "]");
								}
								auto n = va_args.begin();
								auto splt = Rule(*n, this).split;
								auto sep_splt = Rule(sep, this).split;
								temp_split.insert(temp_split.end(), splt.begin(), splt.end());
								n++;
								for (; n != va_args.end(); n++) {
									temp_split.insert(temp_split.end(), sep_splt.begin(), sep_splt.end());
									splt = Rule(*n, this).split;
									temp_split.insert(temp_split.end(), splt.begin(), splt.end());
								}
							}
							else if (i == "__def__") {
								temp_split.push_back(def.name);
							}
							else if (i == "__bracket_beg__") {
								temp_split.push_back(def.bracket.begin);
							}
							else if (i == "__bracket_end__") {
								temp_split.push_back(def.bracket.end);
							}
							else if (i == "__arg_count__") {
								temp_split.push_back(Word(to_string(args.size()), word::number));
							}
							else if (i == "__arg_at") {
								it++;
								str at;
								if (*it == "(")
									it++, go_end(it, at, ")");
								else
									at = *it;
								uint start_pos = 0;
								while ((start_pos = at.find("__arg_count__", start_pos)) != str::npos) {
									at.replace(start_pos, 13, to_string(args.size()));
									start_pos += to_string(args.size()).length();
								}
								temp_split.push_back(args[(uint)te_interp(at.c_str(), 0)]);
							}
							else {
								temp_split.push_back(i);
							}
						}
					}
				}
			}
			else
				temp_split.push_back(i);
		}
		split = temp_split;
		temp_split.clear();
		// 4th iterate
		for (auto it = split.begin(); it != split.end(); it++) {
			const auto& i = *it;
			const auto& it1 = it + 1;
			if (i.front() == '`') {
				temp_split.push_back("R\"__cxx_rule("s + str(i.begin() + 1, i.end() - 1) + ")__cxx_rule\"");
			}
			else if (i.starts_with("f\"")) {
				temp_split.push_back(parse_fliteral(str(i.begin() + 2, i.end() - 1)));
			}
			else if (is_in(i, rbracket)) {
				temp_split.push_back(i);
				if (*(it + 1) == "(") {
					auto cit = it + 2;
					str c;
					go_end(cit, c, ")");
					if (*(cit + 1) == "{")
						continue;
				}
				str t = "(";
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
					t += *it;
					if (*it == "{" or (*it == "}" and *(it + 1) != ";") or *(it + 1) == "}" or *it == ";" or it->type != word::op and (it + 1)->type != word::op or it->type == word::keyw or it->type == word::token) t += ' ';
					it++;
				}
				t += "){";
				const auto& splt = Rule(t, this).split;
				temp_split.insert(temp_split.end(), splt.begin(), splt.end());
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
				const auto& splt = Rule("struct { "s + type + " operator()(" + args + ')' + '{' + body + '}' + fn_name + ';', this).split;
				temp_split.insert(temp_split.end(), splt.begin(), splt.end());
			}
			else if (i == "..") {
				temp_split.pop_back();
				if ((it - 1)->type == word::number and (it + 1)->type == word::number) {
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
				it += 2;
				const str& name = "__operator_"s + *it;
				it += 2;
				go_end(it, args, ")");
				it++;
				if (*it == ";") {
					const auto& splt = Rule(rule_space + type + ' ' + name + '(' + args + "); }\n", this).split;
					temp_split.insert(temp_split.end(), splt.begin(), splt.end());
				}
				else {
					it++;
					go_end(it, body, "}");
					const auto& splt = Rule(rule_space + type + ' ' + name + '(' + args + ')' + "{ " + body + " }" + "}\n", this).split;
					temp_split.insert(temp_split.end(), splt.begin(), splt.end());
				}
			}
			else if (is_in(i, user_ops)) {
				const auto& o = *find(user_ops.begin(), user_ops.end(), i);
				if (o.unary) {
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
			}
			else
				temp_split.push_back(i);
		}
		split = temp_split;
		temp_split.clear();
		afterCode.clear();
		if (not is_child) {
			afterCode = "#include <vector>\n#include <string>\n";
			afterCode += R"(namespace std {
	inline string to_string(string s) { return s; }
	inline string to_string(string* s) { return *s; }
}
)";
		}
		uint tab = 0;
		for (auto it = split.begin(); it != split.end(); it++) {
			afterCode += *it;
			if (it->front() == '#')
				afterCode += '\n';
			else if (it + 1 != split.end()) {
				if (it->back() == ';' or it->back() == '{' or it->back() == '}') {
					if (*it == "{") tab++;
					else if (*it == "}") {
						if (*(afterCode.end() - 2) == '\t')
							afterCode.erase(afterCode.end() - 2);
						if (tab != 0) tab--;
					};
					afterCode += '\n';
					for (uint i = 0; i < tab; i++)
						afterCode += '\t';
				}
				else if (!afterCode.ends_with("\n"))
					if (*(it + 1) == "}" or it->type != word::op and (it + 1)->type != word::op or it->type == word::keyw or it->type == word::token)
						afterCode += ' ';
			}
		}
		return afterCode;
	};
	list<Word> split;
	str code, afterCode;
private:
	bool is_child = false;
	uint meta_counter = 0;
	const str& rule_space = "namespace __cxx_rule { ";
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
							if (it == code.end()) { it--; goto cks; }
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
					cks:;
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
				s += conv_s;
				s += Rule(temp, this).afterCode + add_s;
				temp.clear();
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
