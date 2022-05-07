### Rule
C++ parser
```cpp
const str& falanke = R"(
auto main() -> int {
	// comment
	str hello = "Hello";
	std::cout << f"{hello+"wo"}, World." << '\n';
	str falanke = `wow
	thats amazing
string
			literal...`;
	/* amazing comment */
	return 0;
};
)";
auto main() -> int {
	Rule parser(falanke);
	cout << parser;
	return 0;
};
```

```
OUTPUT:
auto main()->int{ // comment
str hello="Hello"; std::cout<<(std::to_string(hello+"wo") + std::string(", World."))<<'\n'; str falanke=R("wow
        thats amazing
string
                        literal...)"; /* amazing comment */ return 0; };
```
