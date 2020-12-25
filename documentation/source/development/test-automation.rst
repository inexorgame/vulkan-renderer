Test automation
===============

- Inexor will use `Google Test <https://github.com/google/googletest>`__ for `automated software testing <https://en.wikipedia.org/wiki/Test_automation>`__ in the future.
- Because Google Test is `available in conan center <https://conan.io/center/gtest>`__, we will be able to add this to our list of dependencies easily.
- Running automatic tests using GitHub actions is not possible for Vulkan features since this requires a graphics card to be present.
- There are some services which offer test automation for rendering, but they are not free.
- The tests would have to run on the developer's machine locally.
