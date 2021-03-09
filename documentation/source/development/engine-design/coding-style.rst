Coding Style
============

The easiest way to get the right format is to use the provided Clang format file at root.

Other styles which cannot be applied automatically are listed below:

- Use ``#pragma once`` as include guards
- Own headers are included with quotes
- Includes are ordered
    - Own headers
    - Third Party Libraries
    - System Libraries
- Use C++17 namespace style ``namespace inexor::vulkan-renderer``
- No ``using <namespace>``
- For default member initialization use brace instead of equal initialization
- Prefer American English over British English
- Use spaces to indent
- Use Linux line ends (ln) in your commits
- Multiline documentation should use /// in each line

Naming Convention
-----------------

In the ``.clang-tidy`` file and search for ``readability-identifier-naming`` to get the naming convention used by this project.

Error handling
---------------

- Use exceptions for error handling, as proposed by the `C++ core guidelines <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-errors>`__.
- More information about why to use exceptions can be found `here <https://isocpp.org/wiki/faq/exceptions>`__.

Get methods
-----------

- Name: **Don't** use prefix ``get_``. Give the get method the same name as the resource it returns.
- For complex types (``std::vector``, ``std::string``), return a const reference.
- Don't ``const`` the return type for simple types (``int``, ``float``), because this prevents move semantics to be applied.
- For simple types (``int``, ``float``), just copy the return value.
- Mark get methods as ``[[nodiscard]]`` in the header file only.
- Mark get methods as ``const``, so they don't change members.
- Do not add documentation for get methods, since it is self-explanatory.
- Keep get methods directly in the header file.
- Do not add ``inline`` since get methods in header files are always inlined.
- The get method should not run any other code, like checking if the value is actually valid. Since we are using RAII, the value to return must be in a valid state anyways.
- Use operator overloading sparingly. Prefer get methods instead.

**Examples:**

.. code-block:: cpp

    [[nodiscard]] const glm::vec3& position() const {
        return m_position;
    }

    [[nodiscard]] float aspect_ratio() const {
        return m_aspect_ratio;
    }


Removed Clang Tidy Checks
-------------------------

bugprone-narrowing-conversions
    Same as ``cppcoreguidelines-narrowing-conversions``
cppcoreguidelines-avoid-magic-numbers
    Alias of ``readability-magic-numbers``
cppcoreguidelines-c-copy-assignment-signature
    Alias of ``misc-unconventional-assign-operator``
cppcoreguidelines-non-private-member-variables-in-classes
    Alias of ``misc-non-private-member-variables-in-classes``
cppcoreguidelines-pro-bounds-array-to-pointer-decay
    Not suitable for this project.
google-readability-todo
    We do not care about any TODO assignments or related issues.
hicpp-explicit-conversions
    Alias of ``google-explicit-constructor``
hicpp-move-const-arg
    Alias of ``performance-move-const-arg``
hicpp-no-array-decay
    Alias of ``cppcoreguidelines-pro-bounds-array-to-pointer-decay``
hicpp-uppercase-literal-suffix
    Alias of ``readability-uppercase-literal-suffix``
llvm-header-guard
    ``#pragma once`` is used.
modernize-use-trailing-return-type
    Trailing return types are not used.
readability-magic-numbers
    Too many places where it would be useless to introduce a constexpr value.
readability-uppercase-literal-suffix
    Just a style preference.
