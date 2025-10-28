Used modern C++ features
========================

The following is a list of features of modern C++ which we use in the engine. We currenly use C++20.

.. list-table:: List of used modern C++ features.
   :header-rows: 1

   * - Used Feature
     - Required C++ Standard
     - Description
   * - `std::span <https://en.cppreference.com/w/cpp/container/span.html>`__
     - `C++20 <https://en.cppreference.com/w/cpp/20.html>`__
     - ``std::span`` can be used as a container which does not own the memory if contains. A ``std::span`` can be filled with a ``std::vector``, a ``std::array``, and even a normal value from which a ``std::span`` is generated. It is very common in Vulkan functions to accept a pointer to some memory and an integer type variable which contains how many elements are behind that pointer. ``std::span`` is a nice and safe abstraction for this.
   * - `std::optional <https://en.cppreference.com/w/cpp/utility/optional.html>`__
     - `C++17 <https://en.cppreference.com/w/cpp/17.html>`__
     - It is a common use case that an object either has a value of some sort (say an integer), or is in an invalid state and has no valid value. Here, ``std::optional`` is a great tool for expressing this idea directly in code: A ``std::optional<T>`` is of type ``T`` and can either have a valid value of type T, or ``std::nullopt`` if no valid value is assigned (yet). For signed integers for example, the value ``-1`` is sometimes used to express an undefined or invalid value. However, this is not absolutely clear in the code. ``-1`` could be a valid value very much, and the reader of the code has no clear idea if an invalid value is present, as would be the case with ``std::nullopt``.
   * - `lambda expressions <https://en.cppreference.com/w/cpp/language/lambda.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Lambdas are a great language tool from that allows you to place a piece of code anywhere you want.
   * - `constexpr <https://en.cppreference.com/w/cpp/language/constexpr.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Constant expressions are great for a number of reasons. Most importantly, everything that is ``constexpr`` or ``const`` is thread-safe by default.
   * - `thread_local (thread-local storage, TLS) <https://en.cppreference.com/w/c/language/storage_class_specifiers.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - The ``thread_local`` keyword allows you to have one variable for each thread separately, which inherently avoids race conditions.
   * - `std::move <https://en.cppreference.com/w/cpp/utility/move.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Move semantics is a great tool for enabling the programmer to advise the compiler in what way memory can be re-used instead of making unnecessary topics.
   * - `default member initializer <https://en.cppreference.com/w/cpp/language/data_members.html#Member_initialization>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - A great simple way of initializing members without a constructor list.
   * - `deleted functions <https://en.cppreference.com/w/cpp/language/function.html#Deleted_functions>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Defining that a function is explicitely deleted.
   * - `std::string_view <https://en.cppreference.com/w/cpp/string/basic_string_view.html>`__
     - `C++17 <https://en.cppreference.com/w/cpp/17.html>`__
     - A non-owning view into a string.
   * - `std::this_thread <https://en.cppreference.com/w/cpp/symbol_index/this_thread>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Access to thread functionality.
   * - `auto keyword <https://en.cppreference.com/w/cpp/keyword/auto.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - The auto keyword avoids unnecessary long type statements.
   * - `std::source_location <https://en.cppreference.com/w/cpp/utility/source_location.html>`__
     - `C++20 <https://en.cppreference.com/w/cpp/20.html>`__
     - A great tool for accessing metadata about the source location.
   * - `aggregate initialization <https://en.cppreference.com/w/cpp/language/aggregate_initialization.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - This makes filling Vulkan structs much simpler.
   * - `designated initialization <https://en.cppreference.com/w/cpp/language/aggregate_initialization.html#Designated_initializers>`__
     - `C++20 <https://en.cppreference.com/w/cpp/20.html>`__
     - This makes filling Vulkan structs much simpler.
   * - `<random> <https://en.cppreference.com/w/cpp/header/random.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Standard library function for pseudo-random numbers.
   * - `[[nodiscard]] attribute <https://en.cppreference.com/w/cpp/language/attributes/nodiscard>`__
     - `C++17 <https://en.cppreference.com/w/cpp/11.html>`__
     - Specifies that a return value must not be ignored.
   * - `if with initializer <https://en.cppreference.com/w/cpp/language/if.html>`__
     - `C++17 <https://en.cppreference.com/w/cpp/11.html>`__
     - Very useful for limiting the lifetime of an object to the if statement body.
   * - `std::filesystem <https://en.cppreference.com/w/cpp/filesystem.html>`__
     - `C++17 <https://en.cppreference.com/w/cpp/17.html>`__
     - Used for accessing the filesystem.
   * - `enum class <https://en.cppreference.com/w/cpp/language/enum.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Scoped enums.
   * - `Smart pointers <https://en.cppreference.com/w/cpp/memory.html#Smart_pointers>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - ``std::unique_ptr``, ``std::shared_ptr``, ``std::weak_ptr``.
   * - `std::make_unique <https://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique>`__
     - `C++14 <https://en.cppreference.com/w/cpp/14.html>`__
     - Used to create unique pointers.
   * - `std::make_shared <https://en.cppreference.com/w/cpp/memory/shared_ptr/make_shared>`__
     - `C++14 <https://en.cppreference.com/w/cpp/14.html>`__
     - Used to create shared pointers.
   * - `nullptr <https://en.cppreference.com/w/cpp/language/nullptr.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - Replacement for ``NULL`` or ``0``.
   * - `range-based for loops <https://en.cppreference.com/w/cpp/language/range-for.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - This makes for loops much easier in many cases.
   * - `std::forward <https://en.cppreference.com/w/cpp/utility/forward.html>`__
     - `C++14 <https://en.cppreference.com/w/cpp/14.html>`__
     - Variadic templates and perfect forwarding.
   * - `std::function <https://en.cppreference.com/w/cpp/utility/functional/function.html>`__
     - `C++11 <https://en.cppreference.com/w/cpp/11.html>`__
     - A generic way of defining an invocable object.
   * - `std::shared_lock <https://en.cppreference.com/w/cpp/thread/shared_lock.html>`__
     - `C++14 <https://en.cppreference.com/w/cpp/14.html>`__
     - Used for synchronization.
   * - `std::format <https://en.cppreference.com/w/cpp/utility/format/format.html>`__
     - `C++20 <https://en.cppreference.com/w/cpp/20.html>`__
     - Great for formatting strings.
   * - `constexpr if <https://en.cppreference.com/w/cpp/language/if.html#Constexpr_if>`__
     - `C++17 <https://en.cppreference.com/w/cpp/17.html>`__
     - Compile-time constexpr if statements.
