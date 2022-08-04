#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <format>
#include "tinyexpr.h"
using namespace std;
using str = string;
inline str ltrim(str s) { s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](auto ch) { return !std::isspace(ch); })); return s; };
inline str rtrim(str s) { s.erase(std::find_if(s.rbegin(), s.rend(), [](auto ch) { return !std::isspace(ch); }).base(), s.end()); return s; };
inline str trim(const str& s) { return rtrim(ltrim(s)); };
class Rule {
public:
	friend class Rule;
	using str = string;
	using uint = unsigned int;
	using byte = unsigned char;
	template <typename T>
	using list = pmr::vector<T>;
	enum class word : byte { no, keyw, op, token, number, lit };
	enum class err : uint { user_error = 100, same_def_defined_multi_times, not_supported_def_literal, unsupported_def_bracket_type, def_argument_require_sep, def_argument_require_arg, undefined_rep_count, cannot_supported_nested_funtion_decl };
	template <err t, typename... Args> void error(const str mes, Args&&... args) { cerr << '#' << uint(t) << ' ' << vformat(mes, make_format_args(args...)) << '\n'; exit(-1); }
	template <err t> void error(const str mes) { cerr << '#' << uint(t) << ' ' << mes << '\n'; exit(-1); }
	enum class warn : uint { user_warning = 100, multiply_defined_macro, multiply_defined_operator, rep_repeated_once_time, cannot_find_correct_def = 201 };
	template <warn t, typename... Args> void warning(const str mes, Args&&... args) { if (wLevel == max_wLevel or (wLevel != 0 and uint(max_wLevel - wLevel) * 100 < uint(t))) cerr << '$' << uint(t) << ' ' << vformat(mes, make_format_args(args...)) << '\n'; }
	template <warn t> void warning(const str mes) { if (wLevel == max_wLevel or (wLevel != 0 and uint(max_wLevel - wLevel) * 100 < uint(t))) cerr << '$' << uint(t) << ' ' << mes << '\n'; }
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
	struct OP : str {
		using str::basic_string;
		OP() = default;
		OP(const OP&) = default;
		OP(const str& s) : str(s) { };
		bool unary = true; // op[args], op() args, op<>... ekle
	};
	struct Macro {
		str name;
		list<Word> value;
	};
	struct bracket_t { str beg, end; };
	struct Def {
		str name;
		bracket_t bracket;
		list<str> args;
		list<str> seps;
		list<Word> val;
		bool variadic = false;
	};
	const list<str> tokens{ "const", "constexpr", "virtual", "static", "inline", "explicit", "friend", "volatile", "register", "short", "long", "signed", "unsigned" };
	list<str> keywords{ "fn", "auto", "return", "break", "case", "catch", "class", "concept", "continue", "decltype", "default", "delete", "do", "else", "if", "enum", "export", "extern", "for", "goto", "namespace", "new", "noexcept", "operator", "private", "public", "protected", "requires", "sizeof", "struct", "switch", "template", "throw", "try", "typedef", "typename", "union", "while" };
	const list<str> rbracket{ "if", "while" };
	list<str> ops{
		"{", "}","[", "]", "(", ")", "<", ">", "=", "+", "-", "/", "*", "%", "&", "|", "^", ".", ":", ",", ";", "?", "@",
		"==", "!=", ">=", "<=", "<<", ">>", "--", "++", "&&", "||", "+=", "-=", "*=", "/=", "%=", "^=", "|=", "&=", "->", "=>", "::", "..", "[[", "]]",
		"<=>", "<<=", ">>=", "...", "==="
	};
	list<OP> user_ops;
	list<Macro> macros;
	list<Def> defs;
	list<lit_pair> lits{
		lit_pair("\"","\"", {'\0','\0'}, 1, 1, 1, 1, 1),
		lit_pair("L\"","\"", {'\0','\0'}, 1, 1, 1, 1, 1),
		lit_pair("u8\"","\"", {'\0','\0'}, 1, 1, 1, 1, 1),
		lit_pair("u16\"","\"", {'\0','\0'}, 1, 1, 1, 1, 1),
		lit_pair("'","'", {'\0','\0'}, 1, 1, 1, 1, 1),
		lit_pair("//","\n", {'\0','\0'}, 1, 0, 1, 0, 0),
		lit_pair("/*","*/", {'\0','\0'}, 0, 0, 1, 1, 0),
		lit_pair("#","\n", {'\0','\0'}, 1, 0, 1, 0, 0),
		lit_pair("$def","{", {'\0','\0'}, 0, 0, 1, 0, 0),
		lit_pair("$if"," ", {'\0','\0'}, 0, 0, 1, 0, 0),
		lit_pair("$","\n", {'\0','\0'}, 1, 0, 1, 0, 0),
		lit_pair("`","`", {'\0','\0'}, 0, 0, 1, 1, 1),
		lit_pair("f\"","\"", {'{','}'}, 1, 1, 1, 1, 1),
		lit_pair("f`","`", {'{','}'}, 0, 0, 1, 1, 1),
		lit_pair("R\"","\"", {'\0','\0'}, 1, 1, 1, 1, 1),
		// maybe later
		// lit_pair("/+","+/", {'\0','\0'})
	};
	list<str> def_seps{ ",", ":", ";", "=>", "?" };
	list<bracket_t> def_brackets{ {"(",")"}, {"[","]"}, {"<",">"}, {"[[","]]"} };
	Rule() = default;
	Rule(const str& _code) : code(_code) { parse(); };
	Rule(const str& _code, Rule* parent) : code(_code), is_child(true) {
		macros = parent->macros;
		defs = parent->defs;
		wLevel = parent->wLevel;
		parse();
		parent->macros = macros;
		parent->defs = defs;
	};
	str parse() {
		const auto is_in = [](auto v, auto l) { for (const auto& i : l) if (v == i) return true; return false; };
		const auto map_list = [](auto& l, const auto f, const auto m) { for (auto& i : l) if (f == i) i = m; };
		const auto is_macro = [&](const auto& v) { for (const auto& i : macros) if (v == i.name) return true; return false; };
		const auto is_def = [&](const auto& v) { for (const auto& i : defs) if (v == i.name) return true; return false; };
		const auto is_digit = [](char c) { return c > char(47) and c < char(58); };
		const auto go_end = [&](auto& it, auto& val, const auto& ch, const auto& end) {
			int bc = 0, cc = 0, sc = 0;
			while (!(bc == 0 and cc == 0 and sc == 0 and *it == ch)) {
				if (*it == "(") bc++;
				else if (*it == ")") bc--;
				else if (*it == "{") cc++;
				else if (*it == "}") cc--;
				else if (*it == "[") sc++;
				else if (*it == "]") sc--;
				val += *it;
				if (it + 1 == end) break;
				if (*it == "{" or (*it == "}" and *(it + 1) != ";") or *(it + 1) == "}" or *it == ";" or it->type != word::op and (it + 1)->type != word::op or it->type == word::keyw or it->type == word::token) val += ' ';
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
			else if (it1 != split.end()) {
				auto& i1 = *it1;
				if (i == "::" or i == "->" or i == ".") {
					if (it != split.begin() and (it != split.begin() and (it - 1)->type == word::no))
						temp_split.back() += i + i1;
					else
						temp_split.push_back(i + i1);
					it++;
				}
				else if (i == "operator" and (i1.type == word::op or (i1.front() == '"' and i1.back() == '"'))) {
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
			if (i.starts_with("$warn ")) {
				warning<warn::user_warning>(Rule(str(i.begin() + 6, i.end()), this).afterCode);
			}
			else if (i.starts_with("$err ")) {
				error<err::user_error>(Rule(str(i.begin() + 5, i.end()), this).afterCode);
			}
			else if (i.starts_with("#redefine ")) {
				const str s = str(i.begin() + 10, i.end() - 1);
				const auto f = std::find_if(s.begin(), s.end(), [](auto ch) { return std::isspace(ch); });
				const str m = str(s.begin(), f);
				const str val = Rule(str(f, s.end()), this).afterCode;
				temp_split.push_back("#ifdef "s + m);
				temp_split.push_back("#undef "s + m);
				temp_split.push_back("#endif"s);
				temp_split.push_back("#define "s + m + ' ' + val);
			}
			else if (i.starts_with("$rep")) {
				const auto splt = Rule(str(i.begin() + 4, i.end()), this).split;
				int rep_start = 0, rep_end = 0;
				list<Word> rep_splt;
				if (splt.front() != "[") {
					if (splt.front() == "(") {
						auto beg = splt.begin();
						str s;
						go_end(beg, s, ")", splt.end());
						int e = 0;
						rep_start = 0;
						rep_end = (int)te_interp(s.c_str(), &e);
						rep_splt = list<Word>(beg, splt.end());
					}
					else {
						if (splt.front().type == word::number)
							rep_start = 0, rep_end = stoi(splt.front());
						else {
							error<err::undefined_rep_count>("wrong rep count [{}]", splt.front());
						}
						rep_splt = list<Word>(splt.begin() + 1, splt.end());
					}
				}
				else {
					auto beg = splt.begin() + 1;
					str s1, s2;
					go_end(beg, s1, ":", splt.end());
					beg++;
					go_end(beg, s2, "]", splt.end());
					beg++;
					int e1 = 0, e2 = 0;
					rep_start = (int)te_interp(s1.c_str(), &e1);
					rep_end = (int)te_interp(s2.c_str(), &e2);
					if (e1 != 0 or e2 != 0) {
						error<err::undefined_rep_count>("wrong rep count [{}]", (e1 != 0) ? s1 : s2);
					}
					rep_splt = list<Word>(beg, splt.end());
				}
				if (rep_start == rep_end) {
					temp_split.insert(temp_split.end(), rep_splt.begin(), rep_splt.end());
					warning<warn::rep_repeated_once_time>("rep repeated once time");
				}
				else if (rep_start < rep_end) {
					for (int i = rep_start; i <= rep_end; i++) {
						const str i_str = to_string(i);
						auto copy_l = rep_splt;
						map_list(copy_l, "__n__", i_str);
						temp_split.insert(temp_split.end(), copy_l.begin(), copy_l.end());
					}
				}
				else {
					for (int i = rep_start; i >= rep_end; i--) {
						const str& i_str = to_string(i);
						auto copy_l = rep_splt;
						map_list(copy_l, "__n__", i_str);
						temp_split.insert(temp_split.end(), copy_l.begin(), copy_l.end());
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
				else {
					*f = macro;
					warning<warn::multiply_defined_macro>("[{}] macro defined multi times", macro.name);
				}
				keywords.push_back(macro.name);
			}
			else if (i.starts_with("$operator ")) {
				const str s = str(i.begin() + 9, i.end());
				if (not is_in(OP{ s, true }, user_ops))
					user_ops.push_back(OP{ s, true });
				else
					warning<warn::multiply_defined_operator>("[{}] operator defined multi times", s);
				keywords.push_back(s);
			}
			else if (i.starts_with("$def ")) {
				str t = str(i.begin() + 5, i.end());
				Def def;
				auto s = split_code(t);
				auto itt = s.begin();
				while (find_if(def_brackets.begin(), def_brackets.end(), [&](auto&& i) { return i.beg == *itt; }) == def_brackets.end() and itt->type != word::lit) {
					def.name += *itt;
					itt++;
				}
				if (itt->type == word::lit) {
					const auto lit = *find_if(lits.begin(), lits.end(), [&](auto&& i) { return itt->starts_with(i.beg) and itt->ends_with(i.end); });
					if (not lit.can_be_def) {
						error<err::not_supported_def_literal>("[{}] used unsupproted literal for def [{} {}]", def.name, lit.beg, lit.end);
					}
					else {
						def.bracket.beg = lit.beg;
						def.bracket.end = lit.end;
						def.args.push_back(str(itt->begin() + lit.beg.length(), itt->end() - lit.end.length()));
					}
				}
				else {
					for (const auto& i : def_brackets) {
						if (i.beg == *itt) {
							def.bracket = i;
							break;
						}
					}
					if (def.bracket.beg.empty() or def.bracket.end.empty())
						error<err::unsupported_def_bracket_type>("[{}] unsupported def bracket type {}", def.name, *itt);
					itt++;
					bool c = false;
					for (; *itt != def.bracket.end; itt++) {
						if (not c)
							if (not is_in(*itt, def_seps) and (itt->type != word::op or *itt == "..."))
								def.args.push_back(*itt);
							else
								error<err::def_argument_require_arg>("[{}] unexpected def seperator {}", def.name, *itt);
						else
							if (is_in(*itt, def_seps))
								def.seps.push_back(*itt);
							else
								error<err::def_argument_require_sep>("[{}] unexpected def argument {}", def.name, *itt);
						c = !c;
					}
					if (def.args.back() == "...") {
						def.variadic = true;
						def.args.pop_back();
					}
				}
				it++;
				t.clear();
				if (is_def(def.name)) {
					const auto& f = *find_if(defs.begin(), defs.end(), [&](auto&& i) { return i.name == def.name; });
					if (f.args.size() == def.args.size() and f.seps == def.seps and f.bracket.beg == def.bracket.beg and f.bracket.end == def.bracket.end) {
						error<err::same_def_defined_multi_times>("[{}] same def's defined multi times", def.name);
					}
				}
				go_end(it, t, "}", split.end());
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
				auto old_it = it;
				const str def_name = i;
				list<Def> same_n, tsame_n;
				list<list<Word>> args;
				for (const auto& d : defs) if (d.name == i) same_n.push_back(d);
				it++;
				const bool is_lit = (it->type == word::lit);
				if (is_lit) {
					const auto lit = *find_if(lits.begin(), lits.end(), [&](auto&& i) { return it->starts_with(i.beg) and it->ends_with(i.end); });
					const auto r = Rule(str(it->begin() + lit.beg.size(), it->end() - lit.end.size())).split;
					for (const auto& s : same_n) if (it->starts_with(s.bracket.beg) and it->ends_with(s.bracket.end)) tsame_n.push_back(s);
					same_n = tsame_n;
					tsame_n.clear();
					if (same_n.empty() or same_n.size() != 1) {
						warning<warn::cannot_find_correct_def>("[{}] cannot find correct def", def_name), it = old_it, temp_split.push_back(i);
						continue;
					}
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
								temp_split.push_back(def.bracket.beg);
							}
							else if (i == "__bracket_end__") {
								temp_split.push_back(def.bracket.end);
							}
							else
								temp_split.push_back(i);
						}
					}
				}
				else {
					for (const auto& s : same_n) if (s.bracket.beg == *it) tsame_n.push_back(s);
					same_n = tsame_n;
					tsame_n.clear();
					if (same_n.empty()) {
						warning<warn::cannot_find_correct_def>("[{}] cannot find correct def", def_name), it = old_it, temp_split.push_back(i);
						continue;
					}
					const auto& bracket = same_n.back().bracket;
					it++;
					int bc = 0, cc = 0, sc = 0, tc = 0;
					list<Word> t;
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
						if (bc == 0 and cc == 0 and sc == 0 and tc == 0 and not is_in(*it, def_seps)) {
							t.push_back(*it);
						}
						else if (is_in(*it, def_seps)) {
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
					if (same_n.empty() or same_n.size() != 1) {
						warning<warn::cannot_find_correct_def>("[{}] cannot find correct def", def_name), it = old_it, temp_split.push_back(i);
						continue;
					}
					else {
						const auto& def = same_n.back();
						struct arg_t : str { using str::basic_string; arg_t() = default; arg_t(const arg_t&) = default; arg_t(const str& s) : str(s) { }; list<Word> value; };
						list<arg_t> f_args;
						auto arg_it = args.begin();
						auto va_args = list<list<Word>>(args.begin() + def.args.size(), args.end());
						arg_t t_arg;
						for (const auto& i : def.args) {
							t_arg = i;
							t_arg.value = *arg_it;
							arg_it++;
							f_args.push_back(t_arg);
						}
						for (auto itt = def.val.begin(); itt != def.val.end(); itt++) {
							const auto& i = *itt;
							if (i.type != word::op and is_in(i, f_args)) {
								auto splt = find(f_args.begin(), f_args.end(), i)->value;
								temp_split.insert(temp_split.end(), splt.begin(), splt.end());
							}
							else if (def.variadic and not va_args.empty() and i == "..." and *(itt - 1) == "[") {
								temp_split.pop_back();
								itt++;
								list<Word> sep;
								if (*itt == "]")
									sep.push_back(Word("," , word::op));
								else {
									while (*itt != "]" and itt != def.val.end()) {
										sep.push_back(*itt);
										itt++;
									}
								}
								auto n = va_args.begin();
								temp_split.insert(temp_split.end(), n->begin(), n->end());
								n++;
								for (; n != va_args.end(); n++) {
									temp_split.insert(temp_split.end(), sep.begin(), sep.end());
									temp_split.insert(temp_split.end(), n->begin(), n->end());
								}
							}
							else if (i == "__def__") {
								temp_split.push_back(def.name);
							}
							else if (i == "__bracket_beg__") {
								temp_split.push_back(def.bracket.beg);
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
									it++, go_end(it, at, ")", split.end());
								else
									at = *it;
								uint start_pos = 0;
								while ((start_pos = at.find("__arg_count__", start_pos)) != str::npos) {
									at.replace(start_pos, 13, to_string(args.size()));
									start_pos += to_string(args.size()).length();
								}
								const auto arg_at = args[(uint)te_interp(at.c_str(), 0)];
								temp_split.insert(temp_split.end(), arg_at.begin(), arg_at.end());
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
			if (i.front() == '`' and i.back() == '`') {
				temp_split.push_back("R\"__cxx_rule("s + str(i.begin() + 1, i.end() - 1) + ")__cxx_rule\"");
			}
			else if (i.starts_with("f\"")) {
				parse_fliteral_format(str(i.begin() + 2, i.end() - 1), temp_split);
			}
			else if (i.starts_with("f`")) {
				parse_fliteral_format("R\"__cxx_rule("s + str(i.begin() + 2, i.end() - 1) + ")__cxx_rule\"", temp_split);
			}
			else if (is_in(i, rbracket)) {
				temp_split.push_back(i);
				if (*it1 == "(") {
					auto cit = it1 + 1;
					str c;
					go_end(cit, c, ")", split.end());
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
				go_end(it, args, ")", split.end());
				it++;
				if (*it == "->") it++, type = *it, it++;
				it++;
				if (*it == ";") {
					error<err::cannot_supported_nested_funtion_decl>("cannot supported nested function definition, only declaration");
				}
				else {
					go_end(it, body, "}", split.end());
					const auto& splt = Rule("const auto "s + fn_name + " = [&](" + args + ')' + " -> " + type + '{' + body + "};", this).split;
					temp_split.insert(temp_split.end(), splt.begin(), splt.end());
				}
			}
			else if (i == "..") {
				temp_split.pop_back();
				temp_split.push_back("__cxx_rule::__dotdot_op("s + *(it - 1) + ',' + *(it + 1) + ')');
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
				go_end(it, args, ")", split.end());
				it++;
				if (*it == ";") {
					const auto& splt = Rule(rule_space + type + ' ' + name + '(' + args + "); }\n", this).split;
					temp_split.insert(temp_split.end(), splt.begin(), splt.end());
				}
				else {
					it++;
					go_end(it, body, "}", split.end());
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
	}
	list<Word> split;
	str code, afterCode;
	uint warningLevel(uint level) {
		if (level > max_wLevel) level = max_wLevel;
		wLevel = level;
		return wLevel;
	}
private:
	const uint max_wLevel = 3;
	uint wLevel = max_wLevel;
	bool is_child = false;
	uint meta_counter = 0;
	const str& rule_space = "namespace __cxx_rule { ";
	list<Word> split_code(const str& code) {
		const auto& is_in = [](auto v, auto l) { for (const auto& i : l) if (v == i) return true; return false; };
		const auto& is_digit = [](char c) { return c > char(47) and c < char(58); };
		sort(ops.begin(), ops.end(), [](const str& first, const str& second) { return first.size() > second.size(); });
		sort(lits.begin(), lits.end(), [](const auto& first, const auto& second) { return first.beg.size() > second.beg.size(); });
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
		const auto& is_in = [](auto v, auto l) { for (const auto& i : l) if (v == i) return true; return false; };
		str s = str_t + "(\"", temp;
		const str& conv_s = "\") + "s + converter + '(';
		const str& add_s = ") + "s + str_t + "(\"";
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
	list<Word>& parse_fliteral_format(const str code, auto& splt) {
		list<list<Word>> args;
		str s, t;
		for (auto it = code.begin(); it != code.end(); it++) {
			const auto& i = *it;
			if (i == '{') {
				it++;
				int bc = 0, cc = 0, sc = 0;
				while (!(bc == 0 and cc == 0 and sc == 0 and *it == '}')) {
					if (*it == '(') bc++;      else if (*it == ')') bc--;
					else if (*it == '{') cc++; else if (*it == '}') cc--;
					else if (*it == '[') sc++; else if (*it == ']') sc--;
					t += *it;
					it++;
				}
				args.push_back(Rule(t, this).split);
				t.clear();
				s += "{}";
			}
			else {
				s += i;
			}
		}
		if (args.empty()) {
			splt.push_back(Word("\""s + s + '"', word::lit));
		}
		else {
			splt.push_back(Word("format", word::no));
			splt.push_back(Word("(", word::op));
			splt.push_back(Word("\""s + s + '"', word::lit));
			splt.push_back(Word(",", word::op));
			auto n = args.begin();
			splt.insert(splt.end(), n->begin(), n->end());
			n++;
			while (n != args.end()) {
				splt.push_back(Word(",", word::op));
				splt.insert(splt.end(), n->begin(), n->end());
				n++;
			}
			splt.push_back(Word(")", word::op));
		}
		return splt;
	}
};
ostream& operator<<(ostream& os, const Rule& rule) { for (auto i : rule.split) { os << i << '\n'; }; return os; };
