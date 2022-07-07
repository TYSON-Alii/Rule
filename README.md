### Rule [C++ Parser, Re-Writer]
features;
- [x] [f-string literal](https://github.com/TYSON-Alii/Rule/blob/main/README.md#f-string-literal-string-formatting)
- [x] [.. operator](https://github.com/TYSON-Alii/Rule/blob/main/README.md#-operator-range-operator)
- [ ] auto specifier after const keyword
- [x] [#redefine](https://github.com/TYSON-Alii/Rule/blob/main/README.md#redefine-safer-macro-definition)
- [ ] #indefine
- [ ] @ operator
- [ ] #wdefine (weak define)
- [x] [\` \` string literal](https://github.com/TYSON-Alii/Rule/blob/main/README.md#--string-literal-raw-string-literal)
- [x] [defs](https://github.com/TYSON-Alii/Rule/blob/main/README.md#defs-safe-functional-and-overloadable-macros)
- [x] [$rep, $rep[beg:end]](https://github.com/TYSON-Alii/Rule/blob/main/README.md#reprep-rewrites-n-times)
- [x] [safe macros ($macro)](https://github.com/TYSON-Alii/Rule/blob/main/README.md#macro-safe-macros)
- [ ] local macros
- [x] [removing parent bracket requiment](https://github.com/TYSON-Alii/Rule/blob/main/README.md#no-parent-bracket-requiremnt)
- [ ] => operator for lambda
- [ ] infile keyword
- [x] [$operator](https://github.com/TYSON-Alii/Rule/blob/main/README.md#operator-create-unary-operator)
- [ ] typespace
- [ ] : and . namespace operators (std:cout or std.cout)
- [x] [nested functions](https://github.com/TYSON-Alii/Rule/blob/main/README.md#nested-functions)
- [ ] $try, $catch, $endtry
- [ ] function, variable aliasing
## Docs
#### f-string literal _[string formatting]_
```py
hello = "Hello";
f"{hello}, World."
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
```
#### $rep/$rep[:] _[rewrites n times]_
```cpp
cout
$rep 5 << __n__ << ", " // __n__ is order
// prints '0, 1, 2, 3, 4, 5'

$rep[12:14] cout << "num: __n__";
// prints 'num: 12', 'num: 13' and 'num: 14' 
```
#### defs _[safe, functional and overloadable macros]_

* 3 brackets options '(), <> and []'
```cpp
$def foo( /*args*/ ) { /*your code here*/ }
$def foo<> { } // accept
$def foo[] { } // accept
```
* 3 separator options ',', ':' and ';'
```cpp
$def bar(arg1,arg2:arg3) { }
$def bar<arg1:arg2> { }
$def bar[arg1;arg2;arg3,arg4] { }
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
foo() // no error
$def foo(arg1,arg2) { }
$def foo(arg1:arg2) { }
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
#### $macro _[safe macros]_
```cpp
$macro pi 3
$macro pi 3.14 // accept
$macro math.pi 3.1415 // also math::pi and math->pi accept
```
#### no parent bracket requiremnt
```rs
if true or false {

}
while true {

}
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
```cpp
const str& falanke = R"(
$operator falan
$operator filan
$operator echo

void operator falan(auto v) {
	cout << "falanke filanke: " << v << '\n';
}
int operator filan(int v); // declaration
inline auto operator echo(auto v) { return cout << v << '\n'; }

#redefine M_PI 3.14f
$macro math.pi 4 // also math::pi and mat->pi accepted
$macro math.pi 3.14
$macro 3 "its cursed number."

// brackets ( ), [ ], < >
// separators ',', ':', ';'
$def log[type:message] {
	std::cerr << f"{enum_name(type)}:{message}" << '\n';
}
$def log<mes> {
	throw std::runtime_error(mes)
}
$def print[...] { cout << [... << ' ' <<] }
$def average(...) { (([... +]) / __arg_count__) }
$def err<err_type:first_mes,...> {
	cerr << err_type << ':' << first_mes << ' ' << [... << ' ' <<]
}

auto main() -> int {
	// comment
	print[1,selam,"meaba"];
	average(1,2,3,4,5);
	err<"error":"check this after", "line:", 5>;
	str hello = "Hello";
	enum class log_type { info, error, warning };
	log[log_type::error:"oops.."];
	std::cout << f"{hello+f"wow {math.pi}."}, World." << '\n';
	log<3>;
	str falanke = `C:\wow\amazing`;
	int begin = 10, end = 21;
	for (auto&& i : beg..end) falan filan i;
	if 2 + 2 == 4 { // require curly brackets
		echo "evet.";
	}
	/* amazing comment */
	fn func(int wow) {
		if true or false {
			cout << `falanke filanke\n`;
		}
		fn func() {
			$rep 3 "__n__ wow";
		}
	}
	$rep[31:30+math.pi] func(__n__);
	return 0;
}
int operator filan(int v) {
	cout << "oyle iste: " << v << '\n';
	return v + 42;
}
)";
auto main() -> int {
	Rule parser(falanke);
	cout << parser.afterCode;
	return 0;
};
```

```cpp
// OUTPUT:
#include <vector>
#include <string>
namespace std {
        inline string to_string(string s) { return s; }
        inline string to_string(string* s) { return *s; }
}
namespace __cxx_rule{
        void __operator_falan(auto v){
                cout<<"falanke filanke: "<<v<<'\n';
        }
}
namespace __cxx_rule{
        int __operator_filan(int v);
}
namespace __cxx_rule{
        inline auto __operator_echo(auto v){
                return cout<<v<<'\n';
        }
}
#ifdef M_PI
#undef M_PI
#endif
#define M_PI 3.14
auto main()->int{
        cout<<1<<' '<<selam<<' '<<"meaba";
        ((1+2+"its cursed number."+4+5)/5);
        cerr<<"error"<<':'<<"check this after"<<' '<<"line:"<<' '<<5;
        str hello="Hello";
        enum class log_type{
                info,error,warning }
        ;
        std::cerr<<(std::to_string(enum_name(type)) + std::string(":") + std::to_string(message))<<'\n';
        ;
        std::cout<<(std::to_string(hello+(std::string("wow ") + std::to_string(3.14) + std::string("."))) + std::string(", World."))<<'\n';
        throw std::runtime_error("its cursed number.");
        str falanke=R"__cxx_rule(C:\wow\amazing)__cxx_rule";
        int begin=10,end=21;
        for (auto &&i:__cxx_rule::__dotdot_op(beg,end))__cxx_rule::__operator_falan(__cxx_rule::__operator_filan(i));
        if (2+2==4){
                __cxx_rule::__operator_echo("evet.");
        }
        struct {
                auto operator()(int wow){
                        if (true or false){
                                cout<<R"__cxx_rule(falanke filanke\n)__cxx_rule";
                        }
                        struct {
                                auto operator()(){
                                        "0 wow";
                                        "1 wow";
                                        "2 wow";
                                        "3 wow";
                                }
                        }
                        func;
                }
        }
        func;
        func(31);
        func(32);
        func(33);
        return 0;
}
namespace __cxx_rule{
        int __operator_filan(int v){
                cout<<"oyle iste: "<<v<<'\n';
                return v+42;
        }
        }
```
