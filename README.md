### Rule [C++ Parser, Re-Writer]
* [Features](#features)
* [Docs](#Docs)
* [Simple example](#Example)
* [Write your own Rule (and create new features!)](#Write-your-own-rule)
## features
- [x] [f-string literal](#f-string-literal-string-formatting)
- [x] [.. operator](#-operator-range-operator)
- [x] [auto specifier after const/constexpr keyword](#auto-specifier-after-constconstexpr-keyword)
- [x] [#redefine](#redefine-safer-macro-definition)
- [ ] #indefine
- [ ] @ operator
- [ ] #wdefine (weak define)
- [x] [\` \` string literal](#--string-literal-raw-string-literal)
- [x] [defs](#defs-safe-functional-and-overloadable-macros)
- [x] [$rep, $rep[beg:end]](#reprep-rewrites-n-times)
- [x] [safe macros ($macro)](#macro-safe-macros)
- [ ] local macros
- [x] [@[ ] brackets, convert initializer list to vector](#auto-type-variable-definition-with--operator)
- [x] [removing parent bracket requiment](#no-parent-bracket-requiremnt)
- [ ] => operator for lambda
- [x] [auto type declaration with := operator](#auto-type-variable-definition-with--operator)
- [ ] infile keyword
- [x] [$operator](h#operator-create-unary-operator)
- [ ] typespace
- [ ] : and . namespace operators (std:cout or std.cout)
- [x] [nested functions](#nested-functions)
- [ ] $try, $catch, $endtry
- [ ] function, variable aliasing
## Docs
#### f-string literal _[string formatting]_
```py
hello = "Hello";
f"{hello}, World."
f`C:\{hello}\falan`
```
#### .. operator _[range operator]_
```cpp
for (auto&& i : 2..10)
	cout << i << '\n';
```
#### #redefine _[safer macro definition]_
```cpp
#define pi 4
#redefine pi 3
#redefine pi 3.14f
// no error
```
#### \` \` string literal _[raw string literal]_
```d
`C:\throw\no\error\`
`this
is
multi line
`
```
#### $rep/$rep[:] _[rewrites n times]_
```cpp
cout
$rep 5 << __n__ << ", " // __n__ is order
// prints '0, 1, 2, 3, 4, 5'

$rep[12:14] cout << "num: " <<  __n__;
// prints 'num: 12', 'num: 13' and 'num: 14'
```
#### defs _[safe, functional and overloadable macros]_

* 6 brackets options '(), <>, [], [[ ]], <[ ]>, <| |>'
```cpp
$def foo( /*args*/ ) { /*your code here*/ }
$def foo< > { } // accept
$def foo[ ] { } // accept
$def foo[[ ]] { } // accept
$def foo<[ ]> { } // accept
$def foo<| |> { } // accept
```
* 6 separator options ',', ':', ';', '=>', '..' and '?' 
```cpp
$def bar(arg1,arg2:arg3) { }
$def bar<arg1:arg2> { }
$def bar[arg1;arg2?arg3,arg4] { }
$def bar[[arg1=>arg2]] { }
$def bar<[arg1..arg2:arg3]> { }
```
* string literal like ' ', " " and \` \` etc..
```cpp
$def foo"arg" { "arg" } // same, $def foo"" { __arg__ }
$def foo'' { __arg__ } // accept
$def foo`` { } // accept
// some functions
$def foo"arg" { __arg_no_pp__ }
$def foo"arg" { __arg_eval__ }
```
* block style
```cpp
$def once {
	{ static const auto Once = [&]() { __arg__ return nullptr; }(); }
}
int i = 0;
while (true) {
	once {
		cout << i << '\n';
	}
	i++;
}
```
* strong names :: -> .
```cpp
$def foo::bar() { } // accept
$def bar->foo[] { } // accept
$def boo.bom<> { }  // accept
```
* overloadable
```cpp
$def foo() { }
$def foo<> { }
$def foo[] { }
$def foo'' { }
$def foo"" { }
foo() // no error
$def foo(arg1,arg2) { }
$def foo(arg1:arg2) { }
$def foo(arg1=>arg2) { }
foo(1:2) // no error
$def foo[arg1;arg2] { }
$def foo[arg3;arg4] { }
foo[1;2] // error..
```
* variadic
```cpp
$def bar(...) { [...] }
bar(1,2,3,4) // convert -> 1,2,3,4

$def bar(...) { [... sep] }
bar(1,2,3,4) // convert -> 1 sep 2 sep 3 sep 4

$def print(...) { cout [... << ' ' <<] }
print(3,"selam",5.7,true) // convert -> cout << 3 << ' ' << "selam" << ' ' << 5.7 << ' ' << true

// the final separator determines the separator of the variadics
$def foo(separator_comma,...) { }
foo(1,2,3,4)
$def foo(separator_colon:...) { }
foo(1:2:3:4)
$def foo(separator_semicolor;...) { }
foo(1;2;3;4)

$def log<err_type:first_arg,...> {
	cerr << err_type << ':' << first_arg << ' ' << [... << ' ' <<]
}
log<log_type::warn:"this is warning", "line at:", 42>

$def mes[arg1,arg2:...] {
	cout << "def name is: " << __def__ << '\n';
	cout << "bracket type: " << __bracket_beg__ << __bracket_end__ << '\n';
	cout << "arg count: " << __arg_count__ << '\n';
	cout << "first arg: " << __arg_at 0 << '\n';
	cout << "last arg: " << __arg_at (__arg_count__ - 1) << '\n';
}
mes[1, "sss":6.9:kk]
// prints
/*
def name is: mes
bracket type: []
arg count: 4
first arg: 1
last arg: kk
*/
```
* some examples
```cpp
$def print[...] { cout << [... << ' ' <<] }
$def average(...) { (float([... +]) / (float)__arg_count__) }
$def err<err_type:first_mes,...> { cerr << err_type << ':' << first_mes << ' ' << [... << ' ' <<] }
$def for[it_name : list] { for(auto it_name = list.begin(); it_name != list.end(); it_name++) }
$def list.map(list : find => make) { for (auto& i : list) if i == find { i = make; } }
$def if<_if ? _con : _else> { ([&]() { if _con { return _if; } return _else; }()) }
```
#### $macro _[safe macros]_
```cpp
$macro pi 3
$macro pi 3.14 // accept
$macro math.pi 3.1415 // also math::pi and math->pi accept
```
#### auto specifier after const/constexpr keyword
```js
// const num = 52;
constexpr num = 52;
const* nptr = new int(2);
const[r,g,b] = rgb_color;
const&[key,val] = my_pair;
const func = [&](str) { /* falan filan */ };
func(""s);
```
#### no parent bracket requiremnt
```rs
if true or false {

}
while true {

}
```
#### @\[ ] _[convert initializer list to std::vector]_
```cpp
@<bool>[0,0,1,1,1,0,1];
for (const& i : @[1,4,1,5,7])
	echo i;
 ```
#### auto type variable definition with := operator
```cpp
range_list := range[2 .. 5, 1];
// same as 'auto range_list = range[2 .. 5, 1];'
&ref_rlist := range_list;
&&move_rlist := range_list;
*ptr_rlist :=  range_list;
&[key, value] := my_pair;
```
#### $operator _[create unary operator]_
```cpp
$operator echo
auto operator echo(auto&& any_thing) { cout << any_thing; }
echo "Heloo";
echo 74;
$operator sqr
auto operator sqr(int num) { return num * num; }
sqr sqr num; // works (operation priority right to left)
```
#### nested functions
```cpp
int main() {
	fn my_priv_func() {
		fn extra_private() {
			fn yes_not_enough() {
			
			}
		}
	}
	// fn foo(); // not working function definition, only declaration
}
```
## Example
```cxx
// test.cxx file
// $import __cxx_utility.hxx included automaticly
$operator falan
$operator filan

void operator falan(auto v) {
	cout << "falanke filanke: " << v << '\n';
}
int operator filan(int v); // declaration

#redefine M_PI 3.14f
$macro math.pi 4 // also math::pi and math->pi accepted
$macro math.pi 3.14
$macro 42 "did you mean 'everthing'??"

// brackets ( ), [ ], < >, [[ ]], <[ ]>
// separators ',', ':', ';', '=>', '?', '..'
$def log[type:message]{
	std::cerr << f"{enum_name(type)}:{message}" << '\n';
}
$def log<mes> {
	throw std::runtime_error(mes)
}
$def average(...) { (float([... +]) / (float)__arg_count__) }
$def err<err_type:first_mes, ...> {
	cerr << err_type << ':' << first_mes << ' ' << [... << ' ' <<]
}
$def log'arg' { // that's signle line, log` ` support multi line
	cout << 'arg' << '\n'
	// same, cout << __arg__ << '\n'
}
auto main() -> int {
	if<"selm" ? true or false : "mrb">;
	// comment
	list<int> l{ 1,1,1,5,6,1,8 };
	list.map(l : 1 => 31);
	print[1, selam, "meaba"];
	average(1, 2, 3, 4, 5);
	r_list := range[2 .. 5, 1];
	&ref_list := r_list;
	[r,g,b] := my_color;
	for (let i : @[1,4,1,5,7])
		echo i;
	err<"error":"check this after", "line:", 5>;
	str hello = "Hello";
	enum class log_type { info, error, warning };
	log[log_type::error : "oops.."];
	log'`hata` "falan"';
	std::cout << f"{hello+f`wow {math.pi}.`}, World." << '\n';
	str falanke = `C:\wow\amazing`;
	int beg = 10, end = 21;
	for (const&& i : beg..end) {
		once{
			echo f"-_- {i}";
		}
		falan filan(i + 1);
	}
	if 2 + 2 == 4 { // require curly brackets
		echo "evet.";
	}
	/* amazing comment */
	fn func(int wow) {
		if true or false {
			cout << `falanke filanke\n`;
		}
		fn func() -> int {
			$rep 3 "wow";
			return 31;
		}
	}
	// const num = 52;
	const* nptr = new int(2);
	const[r,g,b] = rgb_color;
	const&[key,val] = my_pair;
	constexpr num = 52;
	const func = [&](str) { /* falan filan */ };
	func(""s);
	$rep[31:30 + math.pi] func(__n__);
	return 0;
};
int operator filan(int v) {
	cout << "oyle iste: " << v << '\n';
	return v + 42;
}
```
```cpp
// in main.cpp
#include <Rule.hpp>
auto main() -> int {
	Rule cxx("test.cxx", Rule::from_file);
	cout << cxx.afterCode;
	return 0;
}
```

```cpp
// OUTPUT:
$201 [for] cannot find correct def
$201 [for] cannot find correct def
$101 [math.pi] macro defined multi times
$201 [for] cannot find correct def
$201 [for] cannot find correct def
$201 [if] cannot find correct def
$201 [if] cannot find correct def
$201 [if] cannot find correct def
#include <utility>
#include <string>
using std::string;
using std::to_string;
using std::wstring;
#include <iostream>
using std::cout;
using std::cin;
using std::cerr;
#include <sstream>
#include <fstream>
#include <array>
using std::array;
#include <deque>
#include <bitset>
#include <tuple>
using std::tuple;
#include <typeinfo>
#include <format>
#include <algorithm>
#include <execution>
#include <concepts>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <iomanip>
using namespace std::string_literals;
namespace __cxx_rule{
        inline auto __operator_echo(auto v){
                return cout<<v<<'\n';
        }
}
namespace __cxx_rule{
        void __operator_falan(auto v){
                cout<<"falanke filanke: "<<v<<'\n';
        }
}
namespace __cxx_rule{
        int __operator_filan(int v);
}
#ifdef M_PI
#undef M_PI
#endif
#define M_PI 3.14
auto main()->int {
        ([&](){
                if (true or false){
                        return "selm";
                }
                return "mrb";
        }
        ());
        std::pmr::vector<int >l{
                1,1,1,5,6,1,8 };
        (for (auto &i:l)if (i==1){
                i=31;
        }
        );
        cout<<1<<' '<<selam<<' '<<"meaba";
        (float (1+2+3+4+5)/(float )5);
        auto r_list=([&]()->auto {
                vector<decltype (2)>v;
                for (auto i=2;
                i!=5;
                i+=1)v.push_back(i);
                return v;
        }
        ());
        auto &ref_list=r_list;
        auto [r,g,b]=my_color;
        for (const auto i:std::vector{
                1,4,1,5,7 }
        )__cxx_rule::__operator_echo(i);
        cerr<<"error"<<':'<<"check this after"<<' '<<"line:"<<' '<<5;
        std::string hello="Hello";
        enum class log_type{
                info,error,warning };
        std::cerr<<format("{}:{}",enum_name(log_type::error),"oops..")<<'\n';
        ;
        cout<<'`hata` "falan"'<<'\n';
        std::cout<<format("{}, World.",hello+format(,3.14))<<'\n';
        std::string falanke=R"__cxx_rule(C:\wow\amazing)__cxx_rule";
        int beg=10,end=21;
        for (const auto &&i:__cxx_rule::__dotdot_op(beg,end)){
                {
                        static const auto Once=[&](){
                                __cxx_rule::__operator_echo(f"-_- {i}");
                                return nullptr;
                        }
                        ();
                }
                __cxx_rule::__operator_filan((i+1));
        }
        if (2+2==4){
                __cxx_rule::__operator_echo("evet.");
        }
        const auto func=[&](int wow)->auto {
                if (true or false){
                        cout<<R"__cxx_rule(falanke filanke\n)__cxx_rule";
                }
                const auto func=[&]()->int {
                        "wow";
                        "wow";
                        "wow";
                        "wow";
                        return 31;
                };
        };
        const auto *nptr=new int (2);
        const auto [r,g,b]=rgb_color;
        const auto &[key,val]=my_pair;
        constexpr auto num=52;
        const auto func=[&](std::string){
        };
        func("" s);
        func(31);
        func(32);
        func(33);
        return 0;
};
namespace __cxx_rule{
        int __operator_filan(int v){
                cout<<"oyle iste: "<<v<<'\n';
                return v+"did you mean 'everthing'??";
        }
}
```
## Write your own rule
```cpp
class myRule : public Rule {
public:
	myRule(const str& filename) {
		utility_hxx = true;
		parse(filename, Rule::from_file);
	}
	void user_init() {
		// create a string literal
		auto lit = lit_type(
			"/+", "+/",
			false, // back slash
			false); // new line error
		lits.push_back(lit);
		// create a operator
		ops.push_back("+++");
	}
	bool user_loop(list<Word>::iterator& it, list<Word>& code_split, list<Word>& new_split) {
		auto& i = *it; // current word
		if (i.starts_with("/+") and i.ends_with("+/")) {
			const str s = str(i.begin() + 2, i.end() - 2);
			cout << "$ its a comment!! '" << i << "'\n";
			// const auto eval = Rule(s, this); eval with Rule features;
			const auto eval = split_code(s);
			new_split.insert(new_split.end(), eval.begin(), eval.end());
			return true;
		}
		else if (i == "+++") {
			const auto eval = split_code("+= 2");
			new_split.insert(new_split.end(), eval.begin(), eval.end());
			return true;
		}
		else return false;
	}
};
auto main() -> int {
	myRule cxx("test.vxx");
	cout << cxx.afterCode;
	return 0;
}
```
in test.vxx
```cpp
int main() {
	/+ "falanke filanke"; +/
	int a = 1;
	a+++;
	echo a;
}
```
output:
```cpp
...
$ its a comment!! '/+ "falanke filanke"; +/'
...
int main(){
        "falanke filanke";
        int a=1;
        a+=2;
        __cxx_rule::__operator_echo("safsdfsdfa\n");
}
```
