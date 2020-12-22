Regressions
===========

- If something used to work but it's broken after a certain commit, it's not just some random bug.
- It's probably an issue which was introduced by the code change which was submitted.
- It's important for us to keep working features in a stable state.
- You can use ``git bisect`` for tracing of the commit which introduced the bug.
