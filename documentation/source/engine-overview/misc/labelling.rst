GitHub Issue Labeling
=====================

A full list of GitHub issue labels can be found `here <https://github.com/inexorgame/vulkan-renderer/issues/labels>`__.

The labels are written in two parts, separated by a colon. The first part is the label category, the second the name.

.. csv-table:: Categories
    :header: Abbreviation, Name, Description

    cat, category, main category
    feat, feature, specific feature
    prio, priority, priority
    org, organization, organization/ whats the state of this issue
    diff, difficulty, which skill is required to work on this
    plat, platform, platform specific issue

Some labels are mutual to each other like ``org:in progress`` and ``org:on hold``. Also some labels should not be specified on closed issues like ``org:in progress``.

.. note::

    Github will use black as font color if the background is too light.

+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| Preview             | Category | Name          | Description                                                                        | Color   |
+=====================+==========+===============+====================================================================================+=========+
| |cat:benchmark|     | cat      | benchmark     | testing code performance with automated benchmarking                               | #C0E169 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |cat:bug|           | cat      | bug           | bug/error/mistake which limits the program                                         | #FF0000 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |cat:dependency|    | cat      | dependency    | dependency management                                                              | #BFD4F2 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |cat:dev tools|     | cat      | dev tools     | building/ compiling the program, cmake configuration and general development tools | #78A600 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |cat:documentation| | cat      | documentation | documentation                                                                      | #D1D100 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |cat:enhancement|   | cat      | enhancement   | enhancement/requested feature/update of existing features                          | #0025FF |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |cat:performance|   | cat      | performance   | performance                                                                        | #FF6868 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |cat:refactor|      | cat      | refactor      | refactor/clean up/simplifications/etc.                                             | #FF8C00 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |cat:review|        | cat      | review        | review                                                                             | #69D100 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |cat:security|      | cat      | security      | security/ privacy issues                                                           | #B60205 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |cat:testing|       | cat      | testing       | testing                                                                            | #C0E169 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |diff:first issue|  | diff     | first issue   | good first issue to start contributing                                             | #FEF2C0 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |diff:beginner|     | diff     | beginner      | beginner skills required                                                           | #FEF2C0 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |diff:intermediate| | diff     | intermediate  | intermediate skills required                                                       | #FEF2C0 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |diff:advanced|     | diff     | advanced      | advanced skills required                                                           | #FEF2C0 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |diff:expert|       | diff     | expert        | expert skills required                                                             | #FEF2C0 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |org:discussion|    | org      | discussion    | needs further discussion with others                                               | #DDDDDD |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |org:duplicate|     | org      | duplicate     | duplicate issue                                                                    | #DDDDDD |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |org:help wanted|   | org      | help wanted   | help wanted                                                                        | #DDDDDD |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |org:idea|          | org      | idea          | needs further elaboration before it is possible to continue                        | #DDDDDD |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |org:in progress|   | org      | in progress   | somebody is working on this                                                        | #DDDDDD |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |org:invalid|       | org      | invalid       | invalid issue                                                                      | #DDDDDD |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |org:on hold|       | org      | on hold       | on hold, until ...                                                                 | #DDDDDD |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |org:planned|       | org      | planned       | planned/ prepared issue                                                            | #DDDDDD |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |org:third party|   | org      | third party   | this issue depends on a third party project and is out of our hands                | #DDDDDD |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |org:triage|        | org      | triage        | labels have to be specified                                                        | #D9534F |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |org:wontfix|       | org      | wontfix       | wont be fixed, ever                                                                | #DDDDDD |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |feat:ci|           | feat     | ci            | continuous integration                                                             | #D4C5F9 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |feat:concurrency|  | feat     | concurrency   | multithreading, asynchronous events, concurrency                                   | #D4C5F9 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |feat:render graph| | feat     | render graph  | render graph                                                                       | #D4C5F9 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |feat:gui|          | feat     | gui           | graphical user interface                                                           | #D4C5F9 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |feat:input|        | feat     | input         | keybord/mouse input                                                                | #D4C5F9 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |feat:lightning|    | feat     | lightning     | light system                                                                       | #D4C5F9 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |feat:logging|      | feat     | logging       | logging system                                                                     | #D4C5F9 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |feat:octree|       | feat     | octree        | octree, cube computations                                                          | #D4C5F9 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |feat:rendering|    | feat     | rendering     | rendering                                                                          | #D4C5F9 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |feat:settings|     | feat     | settings      | settings                                                                           | #D4C5F9 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |feat:shader|       | feat     | shader        | shaders                                                                            | #D4C5F9 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |feat:texture|      | feat     | texture       | textures                                                                           | #D4C5F9 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |plat:linux|        | plat     | linux         | Linux specific issue                                                               | #FFC683 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |plat:macos|        | plat     | macos         | MacOS specific issue                                                               | #FFC683 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |plat:windows|      | plat     | windows       | Windows specific issue                                                             | #FFC683 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |prio:blocker|      | prio     | blocker       | this issue cannot be moved to a later milestone, also this label cannot be removed | #B60205 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |prio:high|         | prio     | high          | high priority                                                                      | #FF0000 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+
| |prio:low|          | prio     | low           | low priority                                                                       | #AD8D43 |
+---------------------+----------+---------------+------------------------------------------------------------------------------------+---------+

.. |cat:benchmark| image:: https://img.shields.io/badge/-cat:benchmark-C0E169
.. |cat:bug| image:: https://img.shields.io/badge/-cat:bug-FF0000
.. |cat:crash| image:: https://img.shields.io/badge/-cat:crash-FF0000
.. |cat:dependency| image:: https://img.shields.io/badge/-cat:dependency-BFD4F2
.. |cat:dev tools| image:: https://img.shields.io/badge/-cat:dev_tools-78A600
.. |cat:documentation| image:: https://img.shields.io/badge/-cat:documentation-D1D100
.. |cat:enhancement| image:: https://img.shields.io/badge/-cat:enhancement-0025FF
.. |cat:performance| image:: https://img.shields.io/badge/-cat:performance-FF6868
.. |cat:refactor| image:: https://img.shields.io/badge/-cat:refactor-FF8C00
.. |cat:review| image:: https://img.shields.io/badge/-cat:review-69D100
.. |cat:security| image:: https://img.shields.io/badge/-cat:security-B60205
.. |cat:testing| image:: https://img.shields.io/badge/-cat:testing-C0E169
.. |diff:advanced| image:: https://img.shields.io/badge/-diff:advanced-FEF2C0
.. |diff:beginner| image:: https://img.shields.io/badge/-diff:beginner-FEF2C0
.. |diff:expert| image:: https://img.shields.io/badge/-diff:expert-FEF2C0
.. |diff:first issue| image:: https://img.shields.io/badge/-diff:first_issue-FEF2C0
.. |diff:intermediate| image:: https://img.shields.io/badge/-diff:intermediate-FEF2C0
.. |org:discussion| image:: https://img.shields.io/badge/-org:discussion-DDDDDD
.. |org:duplicate| image:: https://img.shields.io/badge/-org:duplicate-DDDDDD
.. |org:future| image:: https://img.shields.io/badge/-org:future-DDDDDD
.. |org:help wanted| image:: https://img.shields.io/badge/-org:help_wanted-DDDDDD
.. |org:idea| image:: https://img.shields.io/badge/-org:idea-DDDDDD
.. |org:in progress| image:: https://img.shields.io/badge/-org:in_progress-DDDDDD
.. |org:invalid| image:: https://img.shields.io/badge/-org:invalid-DDDDDD
.. |org:on hold| image:: https://img.shields.io/badge/-org:on_hold-DDDDDD
.. |org:planned| image:: https://img.shields.io/badge/-org:planned-DDDDDD
.. |org:third party| image:: https://img.shields.io/badge/-org:third_party-DDDDDD
.. |org:triage| image:: https://img.shields.io/badge/-org:triage-D9534F
.. |org:wontfix| image:: https://img.shields.io/badge/-org:wontfix-DDDDDD
.. |feat:ci| image:: https://img.shields.io/badge/-feat:ci-D4C5F9
.. |feat:concurrency| image:: https://img.shields.io/badge/-feat:concurrency-D4C5F9
.. |feat:render graph| image:: https://img.shields.io/badge/-feat:render_graph-D4C5F9
.. |feat:gui| image:: https://img.shields.io/badge/-feat:gui-D4C5F9
.. |feat:input| image:: https://img.shields.io/badge/-feat:input-D4C5F9
.. |feat:lightning| image:: https://img.shields.io/badge/-feat:lightning-D4C5F9
.. |feat:logging| image:: https://img.shields.io/badge/-feat:logging-D4C5F9
.. |feat:octree| image:: https://img.shields.io/badge/-feat:octree-D4C5F9
.. |feat:rendering| image:: https://img.shields.io/badge/-feat:rendering-D4C5F9
.. |feat:settings| image:: https://img.shields.io/badge/-feat:settings-D4C5F9
.. |feat:shader| image:: https://img.shields.io/badge/-feat:shader-D4C5F9
.. |feat:texture| image:: https://img.shields.io/badge/-feat:texture-D4C5F9
.. |plat:linux| image:: https://img.shields.io/badge/-plat:linux-FFC683
.. |plat:macos| image:: https://img.shields.io/badge/-plat:macos-FFC683
.. |plat:windows| image:: https://img.shields.io/badge/-plat:windows-FFC683
.. |prio:blocker| image:: https://img.shields.io/badge/-prio:blocker-B60205
.. |prio:high| image:: https://img.shields.io/badge/-prio:high-FF0000
.. |prio:low| image:: https://img.shields.io/badge/-prio:low-AD8D43
