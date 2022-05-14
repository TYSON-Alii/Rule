### Rule [C++ Parser, Re-Writer]
features;
- [x] f-string literal
- [x] .. operator
- [ ] auto specifier after const keyword
- [x] #redefine
- [x] \` \` string literal
- [ ] advanced macros; [],(),<>, ()[], (){}, ()<>," ",' ', and etc..
- [ ] #rep, #repn, #endrep
- [ ] safe macros
- [ ] local macros
- [x] removing parent bracket requiment
- [ ] => operator for lambda
- [ ] infile keyword
- [x] #operator
- [ ] typespace
- [ ] dotspace
- [ ] nested functions
- [ ] #try, #catch, #endtry
- [ ] function, variable aliasing
```cpp
const str& falanke = R"(
#operator falan
#operator filan
#operator echo

void operator falan(int v) {
	cout << "falanke filanke: " << v << '\n';
}
inline auto operator echo(auto v) { return cout << v << '\n'; }

#redefine M_PI 3.14f

auto main() -> int {
	// comment
	str hello = "Hello";
	std::cout << f"{hello+"wo"}, World." << '\n';
	str falanke = `C:\wow\amazing`;
	int begin = 10, end = 21;
	for (auto&& i : beg..end) falan filan i;
	cuske.ohake = 1.f;
	if 2 + 2 == 4 { // require curly brackets
		echo "evet.";
	}
	/* amazing comment */
	return 0;
};
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
#include <string>
#include <vector>
namespace __cxx_rule {
namespace std {
        inline string to_string(string s) { return s; };
        inline string to_string(string* s) { return *s; };
};
auto __dotdot_op(auto beg, auto end) {
        std::vector<decltype(beg)> list;
        if (beg < end) for (auto i = beg; i < end; i++) list.push_back(i);
        else if (beg > end) for (auto i = beg; i > end; i--) list.push_back(i);
        else list.push_back(beg);
        return list;
};
void __operator_falan();
void __operator_filan();
void __operator_echo();
};

namespace __cxx_rule { void __operator_falan(int v){ cout<<"falanke filanke: "<<v<<'\n';  }}
 inline namespace __cxx_rule { inline auto __operator_echo(auto v){ return cout<<v<<'\n';  }}
 #ifdef M_PI
 #undef M_PI
 #define M_PI 3.14f
 #else
 #define M_PI 3.14f
 #endif
 auto main()->int{
        str hello="Hello";
        std::cout<<(std::to_string(hello+"wo") + std::string(", World."))<<'\n';
        str falanke=R"__cxx_rule(C:\wow\amazing)__cxx_rule";
        int begin=10,end=21;
        for (auto&&i:__cxx_rule::dotdot_op(beg,end))__cxx_rule::__operator_falan(__cxx_rule::__operator_filan(i));
        cuske.ohake=1.f;
        if (2+2==4){
                __cxx_rule::__operator_echo("evet.");
        } return 0;
};
namespace __cxx_rule { int __operator_filan(int v){ cout<<"oyle iste: "<<v<<'\n'; return v+42;  }}
```
