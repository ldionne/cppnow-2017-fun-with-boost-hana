// Copyright Louis Dionne 2016
// Distributed under the Boost Software License, Version 1.0.

#include <boost/hana.hpp>

#include <cassert>
#include <string>
#include <type_traits>
namespace hana = boost::hana;
using namespace std::literals;


int main() {
// sample(mpl)
auto Types = hana::tuple_t<int, void, char, long, void>;

auto NoVoid = hana::remove_if(Types, [](auto t) {
  return hana::traits::is_void(t);
});
// -> hana::tuple_t<int, char, long>

auto Ptrs = hana::transform(Types, [](auto t) {
  return hana::traits::add_pointer(t);
});
// -> hana::tuple_t<int*, void*, char*, long*, void*>
// end-sample


// sample(fusion-tuple)
// tuple
auto tuple = hana::make_tuple(1, 2.2f, "hello"s, 3.4, 'x');
auto no_floats = hana::remove_if(tuple, [](auto const& t) {
  return hana::traits::is_floating_point(hana::typeid_(t));
});

assert(no_floats == hana::make_tuple(1, "hello"s, 'x'));
// end-sample

// sample(fusion-map)
// map
struct a; struct b; struct c;
auto map = hana::make_map(
  hana::make_pair(hana::type<a>{}, 1),
  hana::make_pair(hana::type<b>{}, 'x'),
  hana::make_pair(hana::type<c>{}, "hello"s)
);

assert(map[hana::type<a>{}] == 1);
assert(map[hana::type<b>{}] == 'x');
assert(map[hana::type<c>{}] == "hello"s);
// end-sample
}
