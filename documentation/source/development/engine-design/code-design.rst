Code Design
===========

Literature
----------

The following books inspired Inexor's code design:

- `Bjarne Stroustrup: The C++ Programming Language (4th Edition) <https://www.stroustrup.com/4th.html>`__
- `Scott Meyers: Effective Modern C++ <https://www.oreilly.com/library/view/effective-modern-c/9781491908419/>`__
- `Scott Meyers: Effective C++: 55 Specific Ways to Improve Your Programs and Designs, Third Edition <https://www.oreilly.com/library/view/effective-c-55/0321334876/>`__
- `Scott Meyers: Effective STL <https://www.oreilly.com/library/view/effective-stl/9780321545183/>`__
- `Nicolai M. Josuttis: C++ Move Semantics - The Complete Guide <https://leanpub.com/cppmove>`__
- `Erich Gamma, Richard Helm, Ralph Johnson, John Vlissides: Design Patterns: Elements of Reusable Object-Oriented Software <https://www.oreilly.com/library/view/design-patterns-elements/0201633612/>`__
- `Robert C. Martin: Clean Code: A Handbook of Agile Software Craftsmanship <https://www.oreilly.com/library/view/clean-code-a/9780136083238/>`__
- `Robert C. Martin: The Clean Coder: A Code of Conduct for Professional Programmers <https://www.oreilly.com/library/view/the-clean-coder/9780132542913/>`__
- `Robert C. Martin: Clean Architecture: A Craftsman's Guide to Software Structure and Design, First Edition <https://www.oreilly.com/library/view/clean-architecture-a/9780134494272/>`__
- `Fedor G. Pikus: Hands-On Design Patterns with C++ <https://www.packtpub.com/product/hands-on-design-patterns-with-c/9781788832564>`__
- `Rian Quinn: Advanced C++ Programming Cookbook <https://subscription.packtpub.com/book/programming/9781838559915>`__

General considerations
----------------------

- Organise the code in components.
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
- In the following section, we will list up he entries which are of considerable interest for the Inexor project.
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
- `C.36: A destructor may not fail <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c36-a-destructor-may-not-fail>`__
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
