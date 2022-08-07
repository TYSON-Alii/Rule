#include <Rule.hpp>
auto main() -> int {
	Rule cxx("test.cxx", Rule::from_file);
	cout << cxx.afterCode;
	return 0;
}
