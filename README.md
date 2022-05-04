### Rule
C++ parser
```cpp
const str& falanke = R"(
auto main() -> int {
	// comment
	cout << "Hello, \"World." << '\n';
	/* amazing comment */
	return 0;
};
)";

auto main() -> int {
	const auto& split = split_code(falanke);
	for (const auto& i : split)
		cout << i << '\n';
	return 0;
};
```

```
OUTPUT:
auto
main
(
)
->
int
{
// comment

cout
<<
"Hello, "World."
<<
'\n'
;
/* amazing comment */
return
0
;
}
;
```
