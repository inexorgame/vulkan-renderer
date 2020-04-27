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
- For default member initialization use equal instead of brace initialization
- Prefer american english over british english
- Use spaces to indent
- Use linux line ends (ln) in your commits
- Multiline documentation should use /// in each line

Naming Convention
-----------------

enum
    PascalCase
enum members
    UPPERCASE
enum class
    PascalCase
enum class members
    snake_case
class
    PascalCase
class members
    snake_case
class methods
    snake_case
functions
    snake_case
constexpr
    SNAKE_CASE
namespace
    snake_case
