Console Logging and Logfiles
=============================

* We use `spdlog <https://github.com/gabime/spdlog>`__ for console logging and logfiles.
* Everything that is written to the console is automatically also written to ``vulkan-renderer.log``.
* The logfile ``vulkan-renderer.log`` is completely overwritten upon every start of ``inexor-vulkan-renderer-example``.
* Unit tests and benchmarks (``inexor-vulkan-renderer-tests`` and ``inexor-vulkan-renderer-benchmarks``) do not create any logfiles.

* We use the following format for log messages: ``%Y-%m-%d %T.%f %^%l%$ %5t [%-10n] %v``

    - ``%Y`` is the year
    - ``%m`` is the month (01 to 12)
    - ``%d`` is the day of month (01 to 31)
    - ``%T`` is `ISO 8601 <https://en.wikipedia.org/wiki/ISO_8601>`__ time format (HH:MM:SS)
    - ``%f`` is the microsecond part of the current second
    - ``%^%l%$`` is the `color-coded <https://github.com/gabime/spdlog/wiki>`__ log level
    - ``%5t`` is the thread id formatted to a string of length 5
    - ``[%-10n]`` is the name of the logger, limited to 10 characters
    - ``%v`` is the log message

For more information, check out spdlog's documentation about `custom formatting <https://github.com/gabime/spdlog/wiki/3.-Custom-formatting>`__.

Logging Guidelines
------------------

- Don't use ``std::cout`` or ``printf``, use spdlog instead.
- Use as many log messages as necessary but do not spam the log output with unnecessary entries either.
- Use all log levels as you need it: ``spdlog::trace``, ``spdlog::info``, ``spdlog::debug``, ``spdlog::error``, ``spdlog::critical``.
- You can print variables with spdlog (see `this reference <https://fmt.dev/latest/syntax.html>`__) because it is based on `fmt library <https://github.com/fmtlib/fmt>`__.
- Use direct API calls like ``spdlog::debug("Example text here");`` instead of creating custom logger instance.
