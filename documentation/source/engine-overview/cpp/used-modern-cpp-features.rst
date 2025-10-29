Used Features of Modern C++
===========================

.. list-table::
   :header-rows: 1
   :widths: 20 10 70

   * - **Used Feature**
     - **Required C++ Standard**
     - **Description**

   * - `std::span <https://en.cppreference.com/w/cpp/container/span.html>`__
     - `C++20 <https://en.cppreference.com/w/cpp/20.html>`__
     - ``std::span`` can be used as a container view that does not own the memory it refers to. A ``std::span`` can be constructed from a ``std::vector``, a ``std::array``, or even a raw pointer and a size. It is very common in Vulkan functions to accept a pointer to some memory and an integer specifying the number of elements. ``std::span`` provides a type-safe abstraction for this pattern.

   * - `std::optional <https://en.cppreference.com/w/cpp/utility/optional.html>`__
     - `C++17 <https://en.cppreference.com/w/cpp/17.html>`__
     - ``std::optional`` represents an object that may or may not contain a value. It is a great tool for expressing optional values directly in code. A ``std::optional<T>`` is either engaged (holding a value of type ``T``) or disengaged (holding no value). This is much clearer than using sentinel values like ``-1`` or ``nullptr`` to indicate an invalid or missing value.

   * - `lambda expressions <https://en.cppreference.com/w/cpp/language/lambda.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Lambdas are a great feature that allow you to define small, inline functions anywhere in your code.

   * - `constexpr <https://en.cppreference.com/w/cpp/language/constexpr.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Constant expressions are powerful for a variety of reasons. Most importantly, anything declared as ``constexpr`` or ``const`` is thread-safe by default.

   * - `thread_local (thread-local storage, TLS) <https://en.cppreference.com/w/c/language/storage_class_specifiers.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - The ``thread_local`` keyword allows you to define variables that are unique to each thread, which helps to avoid race conditions.

   * - `std::move <https://en.cppreference.com/w/cpp/utility/move.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Move semantics are an important feature that allow the programmer to optimize performance by transferring resources instead of copying them unnecessarily.

   * - `default member initializer <https://en.cppreference.com/w/cpp/language/data_members.html#Member_initialization>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - A simple and convenient way of initializing member variables directly in their declaration, without needing a constructor initializer list.

   * - `deleted functions <https://en.cppreference.com/w/cpp/language/function.html#Deleted_functions>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Declaring a function as deleted explicitly prevents it from being used.

   * - `std::string_view <https://en.cppreference.com/w/cpp/string/basic_string_view.html>`__
     - `C++17 <https://en.cppreference.com/w/cpp/17.html>`__
     - A non-owning view into a string.

   * - `std::this_thread <https://en.cppreference.com/w/cpp/symbol_index/this_thread>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Provides access to thread-related functionality.

   * - `auto keyword <https://en.cppreference.com/w/cpp/keyword/auto.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - The ``auto`` keyword avoids unnecessary repetition of long or complex type names.

   * - `std::source_location <https://en.cppreference.com/w/cpp/utility/source_location.html>`__
     - `C++20 <https://en.cppreference.com/w/cpp/20.html>`__
     - Provides access to information about the source code location (e.g., file name, line number, function name).

   * - `aggregate initialization <https://en.cppreference.com/w/cpp/language/aggregate_initialization.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Simplifies initializing structs, especially useful for Vulkan structs.

   * - `designated initialization <https://en.cppreference.com/w/cpp/language/aggregate_initialization.html#Designated_initializers>`__
     - `C++20 <https://en.cppreference.com/w/cpp/20.html>`__
     - Allows explicit member initialization using names, making struct initialization clearer and less error-prone.

   * - `<random> <https://en.cppreference.com/w/cpp/header/random.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Standard library facilities for generating pseudo-random numbers.

   * - `[[nodiscard]] attribute <https://en.cppreference.com/w/cpp/language/attributes/nodiscard>`__
     - `C++17 <https://en.cppreference.com/w/cpp/17.html>`__
     - Specifies that the return value of a function should not be ignored.

   * - `if with initializer <https://en.cppreference.com/w/cpp/language/if.html>`__
     - `C++17 <https://en.cppreference.com/w/cpp/17.html>`__
     - Useful for defining and initializing a variable that is only used within the scope of an ``if`` or ``switch`` statement.

   * - `std::filesystem <https://en.cppreference.com/w/cpp/filesystem.html>`__
     - `C++17 <https://en.cppreference.com/w/cpp/17.html>`__
     - Provides facilities for interacting with the file system.

   * - `enum class <https://en.cppreference.com/w/cpp/language/enum.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Defines scoped enumerations, avoiding name clashes and improving type safety.

   * - `Smart pointers <https://en.cppreference.com/w/cpp/memory.html#Smart_pointers>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Used for automatic memory management and avoiding manual ``delete`` calls. Includes ``std::unique_ptr``, ``std::shared_ptr``, and ``std::weak_ptr``.

   * - `std::make_unique <https://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique>`__
     - `C++14 <https://en.cppreference.com/w/cpp/14.html>`__
     - Safely creates and returns a ``std::unique_ptr`` to a new object.

   * - `std::make_shared <https://en.cppreference.com/w/cpp/memory/shared_ptr/make_shared>`__
     - `C++14 <https://en.cppreference.com/w/cpp/14.html>`__
     - Safely creates and returns a ``std::shared_ptr`` to a new object.

   * - `nullptr <https://en.cppreference.com/w/cpp/language/nullptr.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - A type-safe replacement for ``NULL`` or ``0`` when representing a null pointer.

   * - `range-based for loops <https://en.cppreference.com/w/cpp/language/range-for.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Simplifies iteration over containers.

   * - `std::forward <https://en.cppreference.com/w/cpp/utility/forward.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Used in template programming for perfect forwarding of arguments.

   * - `std::function <https://en.cppreference.com/w/cpp/utility/functional/function.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - A general-purpose wrapper for callable objects (functions, lambdas, functors, etc.).

   * - `std::shared_lock <https://en.cppreference.com/w/cpp/thread/shared_lock.html>`__
     - `C++14 <https://en.cppreference.com/w/cpp/14.html>`__
     - Used for shared (read) access in synchronization primitives like ``std::shared_mutex``.

   * - `std::format <https://en.cppreference.com/w/cpp/utility/format/format.html>`__
     - `C++20 <https://en.cppreference.com/w/cpp/20.html>`__
     - Provides type-safe, modern string formatting functionality (similar to Python’s ``format``).

   * - `constexpr if <https://en.cppreference.com/w/cpp/language/if.html#Constexpr_if>`__
     - `C++17 <https://en.cppreference.com/w/cpp/17.html>`__
     - Allows compile-time conditional branching, enabling cleaner and more efficient template code.

   * - `structured binding declaration <https://en.cppreference.com/w/cpp/language/structured_binding.html>`__
     - `C++17 <https://en.cppreference.com/w/cpp/17.html>`__
     - Declare multiple variables at once by decomposing a returned object, such as an array, tuple, pair, or struct.

   * - `concepts <https://en.cppreference.com/w/cpp/language/constraints.html>`__
     - `C++20 <https://en.cppreference.com/w/cpp/20.html>`__
     - Allows template parameters to be constrained with compile-time requirements, making templates safer and easier to read and use.
