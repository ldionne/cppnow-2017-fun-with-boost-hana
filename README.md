## Fun with [Boost.Hana][]

This repository contains my [reveal.js][]-based presentation on Hana for the
[C++Now 2017][] conference.

## Basic usage
Go to https://ldionne.com/cppnow-2017-fun-with-boost-hana or open
`index.html` with your browser (does not work in Chrome). The slides
are also available as a pdf in `slides.pdf`.

## Advanced usage
From the root of the repository,
```sh
npm install
grunt serve &
```

and then connect to `localhost:8000` to view locally.

## Building the code samples

```sh
(mkdir build && cd build && cmake ..)
cmake --build build install-dependencies
cmake --build build
```

<!-- Links -->
[C++Now 2017]: http://cppnow.org
[Boost.Hana]: https://github.com/boostorg/hana
[reveal.js]: https://github.com/hakimel/reveal.js
