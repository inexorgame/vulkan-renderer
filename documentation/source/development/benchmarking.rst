Benchmarking
============

- Inexor will use `Google Benchmark <https://github.com/google/benchmark>`__ in the future.
- Google Benchmark is also `available in conan center <https://conan.io/center/benchmark>`__, just as `Google Test <https://github.com/google/googletest>`__.
- Benchmarks can also not run in GitHub actions since testing Vulkan features would require a graphics card.
- The tests will run locally on the developer's machine along with the tests.
