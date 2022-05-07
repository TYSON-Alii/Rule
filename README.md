### Rule [C++ Parser]
features;
- [x] f-string literal
- [x] .. operator
- [ ] auto specifier after const keyword
- [ ] #redefine
- [x] \` \` string literal
- [ ] advanced macros; [],(),<>, ()[], (){}, ()<>," ",' ', and etc..
- [ ] #rep, #repn, #endrep
- [ ] safe macros
- [ ] local macros
- [ ] removing parent bracket requiment
- [ ] => operator for lambda
- [ ] infile keyword
- [x] #operator
- [ ] typespace
- [ ] dotspace
- [ ] #try, #catch, #endtry
- [ ] function, variable aliasing
```cpp
const str& falanke = R"(
#operator falan

void operator falan(int v) {
	cout << "falanke filanke: " << v << '\n';
};

auto main() -> int {
	// comment
	str hello = "Hello";
	std::cout << f"{hello+"wo"}, World." << '\n';
	str falanke = `wow..`;
	int begin = 100, end = 50;
	for (auto&& i : {0..10, 15, 30, 42}) falan i;
	/* amazing comment */
	return 0;
};
)";
auto main() -> int {
	Rule parser(falanke);
	cout << parser.afterCode;
	return 0;
};
```

```
OUTPUT:
namespace __cxx_rule {
void __operator_falan();
};

void __cxx_rule::__operator_falan(int v){ cout<<"falanke filanke: "<<v<<'\n'; };
auto main()->int{ str hello="Hello"; std::cout<<(std::to_string(hello+"wo") + std::string(", World."))<<'\n'; str falanke=R("wow..)"; int begin=100,end=50; for (auto&&i:{ 0,1,2,3,4,5,6,7,8,9,15,30,42 } )__cxx_rule::__operator_falan(falan); return 0; };
```
