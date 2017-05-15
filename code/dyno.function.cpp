// Copyright Louis Dionne 2017
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <dyno.hpp>

#include <cassert>
#include <functional>
#include <string>
#include <utility>
using namespace dyno::literals;


//
// Example of creating a naive equivalent to `std::function` using Dyno.
// Taken from Dyno's examples.
//

// sample(Callable)
template <typename Signature>
struct Callable;

template <typename R, typename ...Args>
struct Callable<R(Args...)> : decltype(dyno::requires(
  "call"_s = dyno::function<R (dyno::T const&, Args...)>
)) { };

template <typename R, typename ...Args, typename F>
auto const dyno::default_concept_map<Callable<R(Args...)>, F> =
  dyno::make_concept_map(
    "call"_s = [](F const& f, Args ...args) -> R {
      return f(std::forward<Args>(args)...);
    }
  );
// end-sample

// sample(function)
template <typename Signature>
struct function;

template <typename R, typename ...Args>
struct function<R(Args...)> {
  template <typename F = R(Args...)>
  function(F&& f) : poly_{std::forward<F>(f)} { }

  R operator()(Args ...args) const {
    return poly_.virtual_("call"_s)(poly_, std::forward<Args>(args)...);
  }

private:
  dyno::poly<Callable<R(Args...)>> poly_;
};
// end-sample

int main() {
  function<std::string(int)> tostring = std::to_string;
  assert(tostring(1) == "1");
  assert(tostring(2) == "2");
  assert(tostring(3) == "3");
  assert(tostring(-10) == "-10");
}
