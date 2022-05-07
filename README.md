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
- [ ] #operator
- [ ] typespace
- [ ] dotspace
- [ ] #try, #catch, #endtry
- [ ] function, variable aliasing
```cpp
const str& falanke = R"(
auto main() -> int {
	// comment
	str hello = "Hello";
	std::cout << f"{hello+"wo"}, World." << '\n';
	str falanke = `wow..`;
	int begin = 100, end = 50;
	for (const auto& i : begin..end) { };
	for (auto&& i : {0..10, 15, 30, 42}) { };
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
class __cxx_rule { public:
static auto __dotdot_op(auto beg, auto end) {
        std::vector<decltype(beg)> list;
        if (beg < end)
                for (auto i = beg; i < end; i++) list.push_back(i);
        else if (beg > end)
                for (auto i = beg; i > end; i--) list.push_back(i);
        else
                list.push_back(beg);
        return list;
};
};

auto main()->int{ str hello="Hello"; std::cout<<(std::to_string(hello+"wo") + std::string(", World."))<<'\n'; str falanke=R("wow..)"; int begin=100,end=50; for (const auto&i:__cxx_rule::dotdot_op(begin,end)){ }; for (auto&&i:{ 0,1,2,3,4,5,6,7,8,9,15,30,42 } ){ }; return 0; };
```
