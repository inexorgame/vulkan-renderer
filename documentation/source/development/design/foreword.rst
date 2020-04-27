Foreword
========

Inexor's vulkan-renderer is being developed with the following characteristics:

- Try to keep master branch stable.
- Make sure the code is as easy to debug as possible.
- Always test your code, both with google tests and manually.
- Always write code in a way so it can't be used incorrectly. Assume worst-case scenarios and provide extensive error handling in case something goes wrong.
- Use `CMake <https://cmake.org/>`__ as project map generator.
- If a third-party library is used, make sure it is up-to-date, well documented and battle-tested.
- Do not add dependencies to the repository if possible. Use `conan package manager <https://conan.io/>`__ instead. Sadly, not every dependency has a conan package (e.g. vma).
- If you must add a dependency, put it into ``src/third_party/<name>``.
