// Copyright Louis Dionne 2017
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <boost/hana.hpp>
#include <dyno.hpp>
#include <iostream>
namespace hana = boost::hana;


// This is a minimal reimplementation of Dyno from scratch, to make sure
// that what I present in my slides compiles more-or-less fine. I can't
// just put the actual implementation of Dyno in slides because it would
// be too convoluted.


// sample(dsl)
template <typename Signature>
constexpr hana::basic_type<Signature> function{};

struct T;

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

// sample(concept)
template <typename ...Clauses>
struct concept {
  hana::tuple<Clauses...> clauses;
};

template <typename ...Clauses>
constexpr concept<Clauses...> requires(Clauses ...) {
  return {};
}
// end-sample


// sample(concept_map)
template <typename ...Name, typename ...Function>
auto make_concept_map(hana::pair<Name, Function> ...mappings) {
  return hana::make_map(mappings...);
}

template <typename Concept, typename Model>
auto concept_map = make_concept_map();
// end-sample


template <typename Signature>
using erase_signature_t = typename dyno::detail::erase_signature<
  typename dyno::detail::transform_signature<
    Signature, dyno::detail::replace<::T, dyno::T>::template apply
  >::type
>::type;

// sample(vtable_layout)
template <typename Concept>
auto vtable_layout(Concept c) {
  auto erased = hana::transform(c.clauses,
    hana::fuse([](auto name, auto sig) {
      using Signature = typename decltype(sig)::type;

      // 'double (dyno::T const&)' -> 'double (void const*)'
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
    Signature, dyno::detail::replace<::T, dyno::T>::template apply
  >::type;
  return dyno::detail::erase_function<Sig2>(f);
}

// sample(erase_concept_map)
template <typename Concept, typename ConceptMap>
auto erase_concept_map(Concept c, ConceptMap map) {
  auto erased = hana::transform(c.clauses,
    hana::fuse([&](auto name, auto sig) {
      using Signature = typename decltype(sig)::type;
      return hana::make_pair(
        name, erase_function<Signature>(map[name])
      );
    }));

  // 'erased' is a 'tuple<pair<Name, Signature*>>'
  return hana::to_map(erased);
}
// end-sample


// sample(vtable)
template <typename Concept>
class vtable {
  using Map = typename decltype(vtable_layout(Concept{}))::type;
  Map map_;

public:
  template <typename ConceptMap>
  explicit vtable(ConceptMap map)
    : map_{erase_concept_map(Concept{}, map)}
  { }

  template <typename F>
  auto operator[](F f) const {
    return map_[f];
  }
};
// end-sample


// sample(poly)
template <typename Concept>
struct poly {
  template <typename T>
  poly(T t)
    : self_{new T{t}}
    , vtable_{concept_map<Concept, T>} // <= interesting stuff here
  { }

  template <typename F>
  auto operator->*(F f) {
    return [=](auto ...args) {
      return vtable_[f](self_, args...);
    };
  }

private:
  void* self_; // simplification
  vtable<Concept> vtable_;
};
// end-sample


// sample(HasArea)
struct Circle {
  double x, y, radius;
};

struct HasArea : decltype(requires(
  "area"_s = function<double (T const&)>
)) { };

template <>
auto concept_map<HasArea, Circle> = make_concept_map(
  "area"_s = [](Circle const& self) -> double {
    return 3.1415 * (self.radius * self.radius);
  }
);
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
