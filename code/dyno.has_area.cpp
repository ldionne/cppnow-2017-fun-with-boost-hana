// Copyright Louis Dionne 2017
// Distributed under the Boost Software License, Version 1.0.

#include <dyno.hpp>

#include <iostream>
using namespace dyno::literals;


// sample(definition)
struct Circle {
  double x, y, radius;
};

struct HasArea : decltype(dyno::requires(
  "area"_s = dyno::function<double (dyno::T const&)>
)) { };

template <>
auto dyno::concept_map<HasArea, Circle> = dyno::make_concept_map(
  "area"_s = [](Circle const& self) -> double {
    return 3.1415 * (self.radius * self.radius);
  }
);
// end-sample

// sample(usage)
void print_area(dyno::poly<HasArea> shape) {
  std::cout << "This shape has an area of " << shape->*"area"_s();
}

int main() {
  Circle c = {
    /*x:*/0.0,
    /*y:*/0.0,
    /*radius:*/1.0
  };

  print_area(c);
}
// end-sample
