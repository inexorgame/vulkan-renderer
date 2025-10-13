Octree Binary Format Specification
==================================

Comments start with ``//``, everything after for the rest of the line is ignored.

Spaces can be used to improve the readability, but are not required. Spaces are preferred over tabs.

Variable or data type names are case sensitive.

Endianness
----------

Available values are ``little`` and ``big``.
When defining the reserved data type name ``ENDIANNESS``, all other data types who do not define an own endianness are interpreted as this.

.. code-block::

    | ENDIANNESS : little // default value for all non defined

Data Types
----------

Every data type name, can also be used as an variable name, they don't interfere. Data types can be defined the following way ``| <name> : <bit size> - <endianness> // description``, whereas ``- <endianness>`` is optional, if a default endianness is given.
The bit size can be set by a variable too.

.. code-block::

    | Bit : 1 - little // A bit, 0 or 1.
    | uByte : 4 - little // An unsigned byte.
    | uInt : 8 - little // unsigned integer

    // or

    | ENDIANNESS : little
    | Bit : 1 // A bit, 0 or 1.
    | uInt : 8 // unsigned integer

    uInt : mySize = 0
    | myType : mySize // My special type.

Struct Types
------------

If you have compressed data, it can be easily described as a struct ``struct <name> {``.
Members can be access with a dot.

.. code-block::

    struct SpecSummary {
        > uByte (1) : size // Size
        > uByte (1) : spec_a // Specification A
        > uByte (1) : spec_b // Specification B
    }

    > SpecSummary (1) : spec // specification summary

    if (spec.size > 0) {
        read_spec()
    }


Bit Reading & Variables
-----------------------

Each bit interpretation starts with ``>``, whereas variable name and the reinterpreted type are optional. Variables are only valid in their declared scope.
``<counter>`` is a value, how often the data type is read, it will create a list with its content.
If a reinterpreted type is given, it will be reinterpreted as if the amount of single bits was read by the reinterpretation type (``> Bit (3) : uByte`` reinterprets 3 bits into one uByte).
If the reinterpreted type is bigger, it will be filled with 0 regarding to the endianness, to not change the value (e.g. little endianness will fill from the left side). Same with cutting it off.

.. code-block::

    > <data type> (<counter>) <reinterpreted type> : <variable name> // <description>
                              ^^^^^^^^^^^^^^^^^^^^ ^^^^^^^^^^^^^^^^^ optional

    // examples

    > uByte (7) // string identifier: "myimage"
    > uInt (1) : image_size // image size in bytes
    > Bit (3) uByte : red_color // red color

    > Bit (3) uByte // possible but does not make any sense, as the reinterpreted type will not be used.

    uInt : other_size = image_size // declare new variable other_size with content of image_size
    uInt : id = 0 // declare a new variable of type uInt and content 0

Conditions
----------

``if``, ``else if``, ``else``. Available comparisons are ``==``, ``!=``, ``<``, ``>``, ``<=``, ``>=``. Which can be combined with ``&&`` (and) and ``||`` (or).
Brackets are not required around the condition, but can improve the readability.
You cannot compare a list to a number. You have to reinterpret it beforehand or cast it.

.. code-block::

    > Bit (2) uByte : roll_over // roll over simulation type
    uInt : roll_over_int = roll_over
    if (roll_over_int == 0) { // roll over specification 0
        > uInt (1) // roll over specifications
    } else if (uInt(roll_over) == 1) { // roll over specification 1
        > uInt (1) // roll over specifications
        > uInt (1) // more roll over specifications
    } else { // default roll over specification
        > uByte (1) // weight in kg
        > uInt (1) // roll over specifications
    }

A short version for ``else if`` chaining is ``switch``, ``case``. ``default`` will be used if no other case matches.
There is no default fallthrough, to get this behaviour use ``fallthrough``. A ``break`` inside a switch case will break out of it.

.. code-block::

    > Bit (2) uByte : roll_over // roll over simulation type
    uInt : roll_over_int = roll_over
    switch (roll_over_int) {
    case 0: // roll over specification 0
        > uInt (1) // roll over specifications
    case 1: // roll over specification 1
        > uInt (1) // roll over specifications
        > uInt (1) // more roll over specifications
    default: // default roll over specification
        > uByte (1) // weight in kg
        > uInt (1) // roll over specifications
    }

Loops
-----

There are three types of loops: ``for``, range-based ``for`` and a ``while`` loop.
Brackets are not required around the condition, but can improve the readability. You can break out of a loop with the ``break`` keyword.

.. code-block::

    for (id; id <= 3; id++) { // 0..2
        // ...
    }

    for (0..2 : id) { // including both borders 0 and 2
        // ...
    }

    uInt : id = 0
    while (id <= 3) {
        // ...
        id = id + 1
    }

    uInt : id = 0
    while (true) {
        if (id == 3) {
            break
        }
        // ...
        id = id + 1
    }

Functions
---------

Functions can be used to reuse a specific block again. ``def <name>(<parameter>) <return values> {}``

.. code-block::

    def get_cube(uInt : param) uInt, uByte {
        > uInt (1) : first
        > uByte (1) : second
        return first, second
    } // get_cube

    > uInt (1) : rec_first
    > uByte (1) : rec_second
    rec_first, rec_second = get_cube()
