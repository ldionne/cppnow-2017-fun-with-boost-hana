// Copyright Louis Dionne 2017
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <boost/hana.hpp>
#include <dyno.hpp>

#include <cassert>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
namespace hana = boost::hana;


// This is a minimal reimplementation of Dyno from scratch, to make sure
// that what I present in my slides compiles more-or-less fine. I can't
// just put the actual implementation of Dyno in slides because it would
// be too convoluted.


// sample(dsl)
template <typename Signature>
constexpr hana::basic_type<Signature> function{};

struct self;

// end-sample sample(dsl) sample(string)
template <char ...c>
struct string {
  template <typename Signature>
  constexpr auto operator=(Signature sig) const {
    return hana::make_pair(*this, sig);
  }
};
// end-sample sample(dsl)

template <typename CharT, CharT ...c>
constexpr string<c...> operator""_s() {
  return {};
}
// end-sample

struct string_tag;


namespace boost { namespace hana {
  template <char ...c>
  struct tag_of<::string<c...>> {
    using type = ::string_tag;
  };

  template <>
  struct equal_impl<::string_tag, ::string_tag> {
    template <typename X, typename Y>
    static constexpr auto apply(X, Y) {
      return std::is_same<X, Y>{};
    }
  };

  template <>
  struct hash_impl<::string_tag> {
      template <typename String>
      static constexpr auto apply(String const&) {
          return hana::type_c<String>;
      }
  };
}} // end namespace boost::hana

// sample(trait)
template <typename ...Methods>
struct trait_t {
  hana::tuple<Methods...> methods;
};

template <typename ...Methods>
constexpr trait_t<Methods...> trait(Methods ...) {
  return {};
}
// end-sample


// sample(impl)
template <typename ...Name, typename ...Method>
auto make_impl(hana::pair<Name, Method> ...m) {
  return hana::make_map(m...);
}

template <typename Trait, typename T>
auto impl = make_impl();
// end-sample


template <typename Signature>
using erase_signature_t = typename dyno::detail::erase_signature<
  typename dyno::detail::transform_signature<
    Signature, dyno::detail::replace<::self, dyno::T>::template apply
  >::type
>::type;

// sample(vtable_layout)
template <typename Trait>
auto vtable_layout(Trait t) {
  auto erased = hana::transform(t.methods,
    hana::fuse([](auto name, auto sig) {
      using Signature = typename decltype(sig)::type;

      // 'double (T const&)' -> 'double (void const*)'
      using Erased = erase_signature_t<Signature>;

      return hana::type<hana::pair<decltype(name), Erased*>>{};
    }));

  // 'erased' is a 'tuple<type<pair<Name, Signature*>>...>'
  // we return a 'type<map<pair<Name, Signature*>...>>'
  return hana::unpack(erased, hana::template_<hana::map>);
}
// end-sample


template <typename Signature, typename F>
auto erase_function(F f) {
  using Sig2 = typename dyno::detail::transform_signature<
    Signature, dyno::detail::replace<::self, dyno::T>::template apply
  >::type;
  return dyno::detail::erase_function<Sig2>(f);
}

// sample(erase_impl)
template <typename Trait, typename Impl>
auto erase_impl(Trait t, Impl impl) {
  auto erased = hana::transform(t.methods,
    hana::fuse([&](auto name, auto sig) {
      using Signature = typename decltype(sig)::type;
      return hana::make_pair(
        name, erase_function<Signature>(impl[name])
      );
    }));

  // 'erased' is a 'tuple<pair<Name, Signature*>>'
  return hana::to_map(erased);
}
// end-sample


// sample(vtable)
template <typename Trait>
class vtable {
  using Map = typename decltype(vtable_layout(Trait{}))::type;
  Map map_;

public:
  template <typename Impl>
  explicit vtable(Impl impl)
    : map_{erase_impl(Trait{}, impl)}
  { }

  template <typename F>
  auto operator[](F f) const {
    return map_[f];
  }
};
// end-sample


// sample(poly)
template <typename Trait>
struct poly {
  template <typename T>
  poly(T t)
    : self_{new T{t}}
    , vtable_{impl<Trait, T>} // <= interesting stuff here
  { }

  template <typename F>
  auto operator->*(F f) const {
    return [=](auto ...args) {
      return vtable_[f](self_, args...);
    };
  }

private:
  void* self_; // simplification
  vtable<Trait> vtable_;
};
// end-sample


// sample(definition)
struct Circle {
  double x, y, radius;
};

// end-sample sample(definition) sample(HasArea)
struct HasArea : decltype(trait(
  "area"_s = function<double (self const&)>
)) { };
// end-sample sample(definition)

// end-sample sample(definition) sample(HasArea.Circle)
template <>
auto impl<HasArea, Circle> = make_impl(
  "area"_s = [](Circle const& self) -> double {
    return 3.1415 * (self.radius * self.radius);
  }
);
// end-sample


// sample(Callable)
template <typename Signature>
struct Callable;

template <typename R, typename ...Args>
struct Callable<R(Args...)> : decltype(trait(
  "call"_s = function<R (self const&, Args...)>
)) { };

template <typename R, typename ...Args, typename F>
auto const impl<Callable<R(Args...)>, F> = make_impl(
  "call"_s = [](F const& f, Args ...args) -> R {
    return f(std::forward<Args>(args)...);
  }
);
// end-sample

// sample(std_function)
template <typename Signature>
struct std_function;

template <typename R, typename ...Args>
struct std_function<R(Args...)> {
  template <typename F = R(Args...)>
  std_function(F&& f) : poly_{std::forward<F>(f)} { }

  R operator()(Args ...args) const {
    return (poly_->*"call"_s)(std::forward<Args>(args)...);
  }

private:
  poly<Callable<R(Args...)>> poly_;
};
// end-sample



// sample(usage)
void print_area(poly<HasArea> shape) {
  std::cout << "This shape has an area of " << (shape->*"area"_s)();
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

// Cheap way of running unit tests when program starts up
static auto test_std_function = []{
  std_function<std::string(int)> tostring = std::to_string;
  assert(tostring(1) == "1");
  assert(tostring(2) == "2");
  assert(tostring(3) == "3");
  assert(tostring(-10) == "-10");
  return 0;
}();
