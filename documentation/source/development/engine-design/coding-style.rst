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
