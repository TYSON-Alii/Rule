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
#define stringZ(arg) #arg
$def log'arg' { // that's signle line, log` ` support multi line
	cout << stringZ(__arg_no_pp__) << '\n'
}
const func() -> bool {
	return true;
}
$sep in
$def For[i in l] {
	for (auto&& i : l)
}
#include "Game.hpp" as xs
const main() -> int {
	std.size_t i = 0;
	if<"selm" ? true or false : "mrb">;
	// comment
	list<int> l{ 1,1,1,5,6,1,8 };
	list.map(l : 1 => 31);
	For[i in l] {
		echo i;
	}
	print[1, selam, "meaba"];
	average(1, 2, 3, 4, 5);
	r_list := range[2 .. 5, 1];
	&ref_list := r_list;
	[r,g,b] := my_color;
	for (const& i : @[1,4,1,5,7])
		echo i;
	err<"error" : "check this after", "line:", 5>;
	str hello = "Hello";
	enum class log_type { info, error, warning };
	log[log_type::error : "oops.."];
	log'`hata` "falan"';
	std::cout << f"{hello+f`wow {math.pi}.`}, World." << '\n';
	str falanke = `C:\wow\amazing`;
	int beg = 10, end = 21;
	for(let i : beg..end) {
		once{
			echo f"-_- {i}";
		}
		falan filan (i + 1);
	}
	if 2 + 2 == 4 { // require curly brackets
		echo "evet.";
	}
	if (2 + 2 == 5):
		echo "hayir";
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
	math.pi;
	return 0;
};
int operator filan(int v) {
	cout << "oyle iste: " << v << '\n';
	return v + 42;
}
