*************
Engine design
*************

Folder structure
================

- Use lowercase filenames
- Don't use spaces, use underscores.

Source code
-----------

.. code-block:: none

    connector/  «project root»
    ├── .clang-format  «Clang Format configuration»
    ├── .clang-tidy  «Clang Tidy configuration»
    ├── .git-blame-ignore-revs  «git ignore revisions»
    ├── .gitignore  «git ignore»
    ├── .readthedocs.yml  «Read The Docs configuration»
    ├── CHANGELOG.rst
    ├── CMakeLists.txt
    ├── CODE_OF_CONDUCT.md
    ├── conanfile.py  «Conan configuration»
    ├── CONTRIBUTING.md
    ├── LICENSE.md
    ├── README.rst
    ├── .github/  «GitHub templates and action configurations»
    ├── assets/
    │   ├── models/
    │   └── textures/  «textures»
    ├── benchmarks/
    ├── cmake/  «CMake helpers»
    ├── configuration/
    ├── documentation/
    │   ├── CMakeLists.txt  «CMake file for the documentation»
    │   ├── cmake/  «documentation cmake helpers»
    │   └── source/  «documentation source code»
    ├── example/  «example application»
    ├── include/  «header files»
    ├── shaders/
    ├── src/  «source code»
    ├── tests/
    ├── third_party/  «third party dependencies»
    ├── vma-dumps/
    └── vma-replays/


Application
-----------

.. code-block:: none

    vulkan-renderer/  «application root»
    ├── inexor-vulkan-renderer.exe  «executable»
    ├── ...
    ├── assets/
    ├── shaders/
    └── ...

Dependency management
=====================

- In general we try to keep the number of dependencies at a minimum.
- We avoid to add dependencies directly to the project repository because they increase the size of the repository and we have to update them manually.

- Instead, we prefer to use `conan package manager <https://conan.io/>`__ which allows us to get most dependencies from `conan center <https://conan.io/center/>`__.

Conan package manager
---------------------

- The list of currently used dependencies can be found in ``conanfile.py`` in the root folder of the repository.
- You must have installed `CMake <https://cmake.org/>`__ and `conan package manager <https://conan.io/>`__ in oder to download the dependencies automatically from conan center when running CMake.
- For details please read :ref:`building Instructions<BUILDING>` (:ref:`BUILDING Windows`/:ref:`BUILDING Linux`).

Dependency folder
-----------------

- If we really need a dependency which is not yet available through conan center, we add it manually to the ``third_party`` folder.

Criteria for library selection
------------------------------

If we really need a new library, it should be selected based on the following considerations:

- Are you sure we need this library? Can't we solve the problem with C++ standard library or some other library we are already using?
- The library must have an open source license which is accepted by us (see :ref:`Licenses <LICENSES>`).
- It must be in active development.
- It must have a good documentation.
- A sufficient number of participants must have contributed so we can be sure it is reviewed.


Coding style
============

The easiest way to get the right format is to use the provided `clang-format <https://clang.llvm.org/docs/ClangFormat.html>`__ file in the root directory.

Other styles which cannot be applied automatically are listed below:

- Use ``#pragma once`` as include guards
- Own headers are included with quotes
- Includes are ordered as follows
    - Own headers
    - *empty line*
    - Third Party Libraries
    - *empty line*
    - System Libraries
- Use C++17 namespace style ``namespace inexor::vulkan-renderer``
- No ``using <namespace>``
- For default member initialization use brace instead of equal initialization
- Prefer American English over British English
- Use spaces to indent
- Use Linux line ends (ln) in your commits
- Use ``///`` for multiline documentation instead of ``/**/``

Naming convention
-----------------

Open the ``.clang-tidy`` file and search for ``readability-identifier-naming`` to get the naming convention used by this project.

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


Removed clang-tidy checks
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

Code design
===========

Literature
----------

The following books inspired Inexor's code design:

- `Bjarne Stroustrup: The C++ Programming Language (4th Edition) <https://www.stroustrup.com/4th.html>`__
- `Scott Meyers: Effective Modern C++ <https://www.oreilly.com/library/view/effective-modern-c/9781491908419/>`__
- `Scott Meyers: Effective C++: 55 Specific Ways to Improve Your Programs and Designs, Third Edition <https://www.oreilly.com/library/view/effective-c-55/0321334876/>`__
- `Scott Meyers: Effective STL <https://www.oreilly.com/library/view/effective-stl/9780321545183/>`__
- `Nicolai M. Josuttis: C++ Move Semantics - The Complete Guide <https://leanpub.com/cppmove>`__
- `Nicolai M. Josuttis: C++ Templates - The Complete Guide, 2nd Edition <http://www.tmplbook.com/>`__
- `Bartłomiej Filipek C++ Lambda Story <https://leanpub.com/cpplambda>`__
- `Erich Gamma, Richard Helm, Ralph Johnson, John Vlissides: Design Patterns: Elements of Reusable Object-Oriented Software <https://www.oreilly.com/library/view/design-patterns-elements/0201633612/>`__
- `Robert C. Martin: Clean Code: A Handbook of Agile Software Craftsmanship <https://www.oreilly.com/library/view/clean-code-a/9780136083238/>`__
- `Robert C. Martin: The Clean Coder: A Code of Conduct for Professional Programmers <https://www.oreilly.com/library/view/the-clean-coder/9780132542913/>`__
- `Robert C. Martin: Clean Architecture: A Craftsman's Guide to Software Structure and Design, First Edition <https://www.oreilly.com/library/view/clean-architecture-a/9780134494272/>`__
- `Fedor G. Pikus: Hands-On Design Patterns with C++ <https://www.packtpub.com/product/hands-on-design-patterns-with-c/9781788832564>`__
- `Rian Quinn: Advanced C++ Programming Cookbook <https://subscription.packtpub.com/book/programming/9781838559915>`__

General considerations
----------------------

- Organize the code in components.
- Split declarations and definitions, if possible.
- Make appropriate use of the standard library.
- Avoid data redundancy in the engine. Do not keep memory copied unnecessarily.
- Do not duplicate code. Find an appropriate abstraction which accounts for the scenario.
- Try to keep dependencies between components at minimum because single components (e.g. classes) should be as recyclable as possible.
- Use `spdlog <https://github.com/gabime/spdlog>`__ instead of ``printf`` or ``std::cout`` for console output.
- Use ``assert`` to validate parameters or necessary resources during development (debug mode).
- Document the code using `doxygen <http://doxygen.nl/>`__ comments. Code without documentation is almost useless.
- Make sure the code is platform-independent. For now, we will support Windows and Linux but not Mac OS.
- Use `Vulkan memory allocator library <https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator>`__ for Vulkan-specific memory allocations like buffers.
- Do not allocate memory manually. Use modern ++ features like `smart pointers <https://en.cppreference.com/book/intro/smart_pointers>`__ or STL containers instead.
- `Don't use global variables <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i22-avoid-complex-initialization-of-global-objects>`__.
- `Don't use the singleton pattern <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ri-singleton>`__ as it makes thread safety and refactoring difficult.
- Don't use call-by-value for returning values from a function call.
- Don't use macros for code generation or as a replacement for enumerations.

C++ core guidelines
-------------------

- The `C++ code guidelines <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines>`__ are a set of rules to use for modern C++ projects created by the C++ community.
- In the following section, we will list up the entries which are of considerable interest for the Inexor project.
- There will be some gaps in the number as we skipped some of the less importer ones.
- Also the code guidelines have gaps by default (blank space for new rules).

Philosophy
^^^^^^^^^^

- `P.1: Express ideas directly in code <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rp-direct>`__
- `P.3: Express intent <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rp-what>`__
- `P.4: Ideally, a program should be statically type safe <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rp-typesafe>`__
- `P.5: Prefer compile-time checking to run-time checking <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p5-prefer-compile-time-checking-to-run-time-checking>`__
- `P.8: Don’t leak any resources <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#p8-dont-leak-any-resources>`__
- `P.9: Don’t waste time or space <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rp-waste>`__
- `P.10: Prefer immutable data to mutable data <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rp-mutable>`__
- `P.11: Encapsulate messy constructs, rather than spreading through the code <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rp-library>`__

Interfaces
^^^^^^^^^^

- `I.1: Make interfaces explicit <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ri-explicit>`__
- `I.4: Make interfaces precisely and strongly typed <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ri-typed>`__
- `I.5: State preconditions (if any) <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ri-pre>`__
- `I.7: State postconditions <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ri-post>`__
- `I.10: Use exceptions to signal a failure to perform a required task <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ri-except>`__
- `I.11: Never transfer ownership by a raw pointer (T*) or reference (T&) <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ri-raw>`__
- `I.13: Do not pass an array as a single pointer <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ri-array>`__
- `I.23: Keep the number of function arguments low <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ri-nargs>`__
- `I.24: Avoid adjacent parameters of the same type when changing the argument order would change meaning <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ri-unrelated>`__

Functions and class methods
^^^^^^^^^^^^^^^^^^^^^^^^^^^

- `F.1: “Package” meaningful operations as carefully named functions <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-package>`__
- `F.2: A function should perform a single logical operation <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-logical>`__
- `F.4: If a function may have to be evaluated at compile time, declare it constexpr <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-constexpr>`__
- `F.7: For general use, take T* or T& arguments rather than smart pointers <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-smart>`__
- `F.15: Prefer simple and conventional ways of passing information <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-conventional>`__
- `F.16: For “in” parameters, pass cheaply-copied types by value and others by reference to const <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-in>`__
- `F.18: For “will-move-from” parameters, pass by X&& and std::move the parameter <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-consume>`__
- `F.20: For “out” output values, prefer return values to output parameters <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-out>`__
- `F.21: To return multiple “out” values, prefer returning a struct or tuple <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-out-multi>`__
- `F.26: Use a unique_ptr<T> to transfer ownership where a pointer is needed <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-unique_ptr>`__
- `F.27: Use a shared_ptr<T> to share ownership <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-shared_ptr>`__
- `F.43: Never (directly or indirectly) return a pointer or a reference to a local object <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-dangle>`__
- `F.45: Don’t return a T&& <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-return-ref-ref>`__
- `F.51: Where there is a choice, prefer default arguments over overloading <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-default-args>`__
- `F.55: Don’t use va_arg arguments <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#F-varargs>`__

Classes
^^^^^^^

- `C.2: Use class if the class has an invariant; use struct if the data members can vary independently <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-struct>`__
- `C.3: Represent the distinction between an interface and an implementation using a class <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-interface>`__
- `C.4: Make a function a member only if it needs direct access to the representation of a class <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-member>`__
- `C.7: Don’t define a class or enum and declare a variable of its type in the same statement <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-standalone>`__
- `C.8: Use class rather than struct if any member is non-public <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-class>`__
- `C.9: Minimize exposure of members <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-private>`__

Enumerations
^^^^^^^^^^^^

- `Enum.1: Prefer enumerations over macros <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Renum-macro>`__
- `Enum.2: Use enumerations to represent sets of related named constants <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Renum-set>`__
- `Enum.3: Prefer class enums over “plain” enums <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Renum-class>`__
- `Enum.6: Avoid unnamed enumerations <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Renum-unnamed>`__
- `Enum.7: Specify the underlying type of an enumeration only when necessary <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Renum-underlying>`__

Resource management
^^^^^^^^^^^^^^^^^^^

- `R.1: Manage resources automatically using resource handles and RAII (Resource Acquisition Is Initialization) <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-raii>`__
- `R.2: In interfaces, use raw pointers to denote individual objects (only) <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-use-ptr>`__
- `R.3: A raw pointer (a T*) is non-owning <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-ptr>`__
- `R.4: A raw reference (a T&) is non-owning <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-ref>`__
- `R.5: Prefer scoped objects, don’t heap-allocate unnecessarily <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-scoped>`__
- `R.10: Avoid malloc() and free() <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-mallocfree>`__
- `R.11: Avoid calling new and delete explicitly <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-newdelete>`__
- `R.12: Immediately give the result of an explicit resource allocation to a manager object <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-immediate-alloc>`__
- `R.13: Perform at most one explicit resource allocation in a single expression statement <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-single-alloc>`__

Classes
^^^^^^^

- `C.30: Define a destructor if a class needs an explicit action at object destruction <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c30-define-a-destructor-if-a-class-needs-an-explicit-action-at-object-destruction>`__
- `C.31: All resources acquired by a class must be released by the class’s destructor <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c31-all-resources-acquired-by-a-class-must-be-released-by-the-classs-destructor>`__
- `C.35: A base class destructor should be either public and virtual, or protected and non-virtual <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c35-a-base-class-destructor-should-be-either-public-and-virtual-or-protected-and-non-virtual>`__
- `C.36: A destructor must not fail <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-dtor-fail>`__
- `C.40: Define a constructor if a class has an invariant <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c40-define-a-constructor-if-a-class-has-an-invariant>`__
- `C.41: A constructor should create a fully initialized object <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c41-a-constructor-should-create-a-fully-initialized-object>`__
- `C.42: If a constructor cannot construct a valid object, throw an exception <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c42-if-a-constructor-cannot-construct-a-valid-object-throw-an-exception>`__
- `C.43: Ensure that a copyable (value type) class has a default constructor <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c43-ensure-that-a-copyable-value-type-class-has-a-default-constructor>`__
- `C.44: Prefer default constructors to be simple and non-throwing <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c44-prefer-default-constructors-to-be-simple-and-non-throwing>`__
- `C.46: By default, declare single-argument constructors explicit <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c46-by-default-declare-single-argument-constructors-explicit>`__
- `C.47: Define and initialize member variables in the order of member declaration <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c47-define-and-initialize-member-variables-in-the-order-of-member-declaration>`__
- `C.49: Prefer initialization to assignment in constructors <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c49-prefer-initialization-to-assignment-in-constructors>`__
- `C.62: Make copy assignment safe for self-assignment <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c62-make-copy-assignment-safe-for-self-assignment>`__
- `C.64: A move operation should move and leave its source in a valid state <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c64-a-move-operation-should-move-and-leave-its-source-in-a-valid-state>`__
- `C.65: Make move assignment safe for self-assignment <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c65-make-move-assignment-safe-for-self-assignment>`__
- `C.80: Use =default if you have to be explicit about using the default semantics <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c80-use-default-if-you-have-to-be-explicit-about-using-the-default-semantics>`__
- `C.81: Use =delete when you want to disable default behavior (without wanting an alternative) <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c81-use-delete-when-you-want-to-disable-default-behavior-without-wanting-an-alternative>`__
- `C.82: Don’t call virtual functions in constructors and destructors <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c82-dont-call-virtual-functions-in-constructors-and-destructors>`__
- `C.90: Rely on constructors and assignment operators, not memset and memcpy <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c90-rely-on-constructors-and-assignment-operators-not-memset-and-memcpy>`__
- `C.129: When designing a class hierarchy, distinguish between implementation inheritance and interface inheritance <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c129-when-designing-a-class-hierarchy-distinguish-between-implementation-inheritance-and-interface-inheritance>`__
- `C.131: Avoid trivial getters and setters <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c131-avoid-trivial-getters-and-setters>`__
- `C.132: Don’t make a function virtual without reason <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c132-dont-make-a-function-virtual-without-reason>`__
- `C.133: Avoid protected data <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c133-avoid-protected-data>`__

Follow rule of 0 and rule of 5
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- `C.20 If you can avoid defining default operations, do <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-zero>`__
- `C.21: If you define or =delete any copy, move, or destructor function, define or =delete them all <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c21-if-you-define-or-delete-any-copy-move-or-destructor-function-define-or-delete-them-all>`__

Performance
^^^^^^^^^^^

- `Per.1: Don’t optimize without reason <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rper-reason>`__
- `Per.2: Don’t optimize prematurely <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#per2-dont-optimize-prematurely>`__
- `Per.3: Don’t optimize something that’s not performance critical <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#per3-dont-optimize-something-thats-not-performance-critical>`__
- `Per.4: Don’t assume that complicated code is necessarily faster than simple code <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#per4-dont-assume-that-complicated-code-is-necessarily-faster-than-simple-code>`__
- `Per.5: Don’t assume that low-level code is necessarily faster than high-level code <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#per5-dont-assume-that-low-level-code-is-necessarily-faster-than-high-level-code>`__
- `Per.6: Don’t make claims about performance without measurements <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#per6-dont-make-claims-about-performance-without-measurements>`__
- `Per.11: Move computation from run time to compile time <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#per11-move-computation-from-run-time-to-compile-time>`__


Design patterns
===============

- Check out `Refactoring Guru <https://refactoring.guru/design-patterns>`__ to learn more about `software design patterns <https://en.wikipedia.org/wiki/Software_design_pattern>`__.
- `Don't use the singleton pattern <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ri-singleton>`__ as it makes thread safety and refactoring difficult.
- Use the `builder pattern <https://refactoring.guru/design-patterns/builder>`__ for composition of complicated data structures.
- An example of a builder pattern would be the `descriptor builder <https://github.com/inexorgame/vulkan-renderer/blob/master/src/vulkan-renderer/wrapper/descriptor_builder.cpp>`__.

Regressions
===========

- If something used to work but it's broken after a certain commit, it's not just some random bug.
- It's probably an issue which was introduced by the code change which was submitted.
- It's important for us to keep working features in a stable state.
- You can use ``git bisect`` for tracing of the commit which introduced the bug.
