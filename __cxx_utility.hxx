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
$operator echo
inline auto operator echo(auto v) { return cout << v << '\n'; }
$def print[...]{ cout << [... << ' ' << ] }
$def for[it_name:list]{ for (auto it_name = list.begin(); it_name != list.end(); it_name++) }
$def list.map(list : find => make) { (for (auto& i : list) if i == find { i = make; }) }
inline const if_else(bool con, auto _if, auto _else) {
	if _con { return _if; } return _else;
}
$def if<_if ? _con : _else> { if_else(_con, _if, _else) }
$def slice[list, beg .. end] { decltype(list)(list.begin()+beg, list.begin() + end) }
inline auto range_fn(start, end, step) {
	vector<decltype(start)> v;
	for (auto i = start; i < end; i += step) v.push_back(i);
	return v;
}
$def range[start ..end, step] {
	range_fn(start, end, step)
}
$def once { { static const auto Once = [&]() { __arg__ return nullptr; }(); } }
$macro self (*this)
$macro var auto&
$macro let const auto
$macro elif else if
$macro lambda [&]
$macro pub public:
$macro priv private:
$macro global ::
$macro ret return
$macro del delete
$macro real float
$macro wchar wchar_t
$macro bit bool
$macro str std::string
$macro wstr std::wstring
$macro integer int
$macro uint unsigned int
$macro i8  std::int8_t
$macro i16 std::int16_t
$macro i32 std::int32_t
$macro i64 std::int64_t
$macro u8  std::uint8_t
$macro u16 std::uint16_t
$macro u32 std::uint32_t
$macro u64 std::uint64_t
$macro f32 float
$macro f64 double
$macro list std::pmr::vector
$macro ptr auto*
$nspace std
