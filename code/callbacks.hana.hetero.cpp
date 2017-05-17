// Copyright Louis Dionne 2016
// Distributed under the Boost Software License, Version 1.0.

#include <boost/hana.hpp>

#include <functional>
#include <iostream>
#include <vector>
namespace hana = boost::hana;


// sample(dsl)
template <typename Signature>
constexpr hana::basic_type<Signature> function{};

template <char ...c>
struct event {
  template <typename F>
  constexpr auto operator=(F f) const {
    return hana::make_pair(*this, f);
  }
};

template <typename CharT, CharT ...c>
constexpr event<c...> operator""_event() {
  return {};
}
// end-sample

struct event_tag;

namespace boost { namespace hana {
  template <char ...c>
  struct tag_of<::event<c...>> {
    using type = ::event_tag;
  };

  template <>
  struct equal_impl<::event_tag, ::event_tag> {
    template <typename X, typename Y>
    static constexpr auto apply(X, Y) {
      return std::is_same<X, Y>{};
    }
  };

  template <>
  struct hash_impl<::event_tag> {
      template <typename Event>
      static constexpr auto apply(Event const&) {
          return hana::type_c<Event>;
      }
  };
}} // end namespace boost::hana


// sample(struct)
template <typename ...Events>
struct event_system;

template <typename ...Events, typename ...Signatures>
struct event_system<hana::pair<Events, hana::basic_type<Signatures>>...> {
  hana::map<hana::pair<Events, std::vector<std::function<Signatures>>>...> map_;
// end-sample

// sample(on)
template <typename Event, typename F>
void on(Event e, F callback) {
  auto is_known_event = hana::contains(map_, e);
  static_assert(is_known_event,
    "trying to add a callback to an unknown event");

  map_[e].push_back(callback);
}
// end-sample

// sample(trigger)
template <typename Event, typename ...Args>
void trigger(Event e, Args ...a) const {
  auto is_known_event = hana::contains(map_, e);
  static_assert(is_known_event,
    "trying to trigger an unknown event");

  for (auto& callback : map_[e])
    callback(a...);
}
// end-sample
};

// sample(constructor)
template <typename ...Events>
event_system<Events...> make_event_system(Events ...events) {
  return {};
}
// end-sample


// sample(usage)
int main() {
  auto events = make_event_system(
    "foo"_event = function<void(std::string)>,
    "bar"_event = function<void(int)>,
    "baz"_event = function<void(double)>
  );

  events.on("foo"_event, [](std::string s) { std::cout << "foo triggered with '" << s << "'!\n"; });
  events.on("foo"_event, [](std::string s) { std::cout << "foo with '" << s << "' again!\n"; });
  events.on("bar"_event, [](int i) { std::cout << "bar triggered with '" << i << "'!\n"; });
  events.on("baz"_event, [](double d) { std::cout << "baz triggered with '" << d << "'!\n"; });
  // events.on("unknown"_event, []() { }); // compiler error!

  events.trigger("foo"_event, "hello"); // no overhead for event lookup
  events.trigger("bar"_event, 4);
  events.trigger("baz"_event, 3.3);
  // events.trigger("unknown"_event); // compiler error!
}
// end-sample

