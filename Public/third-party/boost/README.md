# Boost header subset

This directory contains the header-only dependency closure used by NewAOE,
extracted from Boost 1.90.0 with `bcp --scan`.

The source headers came from Ubuntu package `libboost1.90-dev`
`1.90.0-6ubuntu1` on 2026-10-24. The upstream release is available from
https://www.boost.org/releases/1.90.0/.

The directly used libraries are Boost.Asio, Boost.Beast, Boost.Interprocess,
Boost.UUID, and Boost.LexicalCast. Their transitive headers for supported
platform branches are included under `boost/`; no compiled Boost libraries are
vendored or required.

When NewAOE adds another Boost include, regenerate the subset by scanning these
project sources plus the new source files:

```sh
bcp --boost=/path/to/boost-root --scan \
  Public/profile/profile_worker.cpp \
  Public/network/foundation/sock.cpp \
  Public/util/util.cpp \
  Public/vec/cast.hpp \
  /path/to/output
```

Boost is distributed under the Boost Software License 1.0; see
`LICENSE_1_0.txt`.
