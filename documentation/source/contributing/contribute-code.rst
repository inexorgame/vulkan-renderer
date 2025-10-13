Contribute Code
===============

- If you want to contribute code, join our `Discord <https://discord.com/invite/acUW8k7>`__ and have a look at our `GitHub repository <https://github.com/inexorgame/vulkan-renderer>`__
- Try to find a `first good issue <https://github.com/inexorgame/vulkan-renderer/issues>`__ in our issue tracker
- Open a `pull request <https://github.com/inexorgame/vulkan-renderer/pulls>`__
- We want to keep commit as small in size as possible

Signing Commits
---------------

- We encourage you to sign your commits, although this is not a strict requirement from our side
- You can find help on how to sign commits in `GitHub's docs <https://docs.github.com/en/github/authenticating-to-github/signing-commits>`__

.. _COMMIT_NAMING:

Commit Naming Convention
------------------------

The commit naming convention will be checked by our continuous integration. In order to be valid, the following rules must all be fulfilled:

- The commit message must begin with a category in square brackets which describes the code part that was changed
- The commit category must not be empty 
- A commit category must contain at least two characters. Allowed characters are: letters ``a-z``, numbers ``0-9``, and ``-``. For example ``[gpu-texture]``
- If two code parts are affected, separate the categories using ``|``, for example ``[gpu-texture|gpu-info]``
- If three or more code parts are affected, use ``[*]`` as category
- Leave one space after the commit category
- The commit message itself must consist of the following characters: letters ``a-z``, numbers ``0-9``
- The commit message must begin with a capital letter
- The commit message must exist and it must not be empty
- The commit message must not end with ``.``, ``!`` or ``?``

**Examples**

- ``[docs] Explain commit naming convention``
- ``[gpu-info] Don't display empty gpu info queries as error``
- ``[gpu-info|representation] Cleanup``
- ``[ci] Add commit naming check``
- ``[*] Move GPU info to vk tools``

**Additional Information**

The regex pattern for the commit category is ``\*|(?:[a-z0-9]{2,}[\s|-]?)+``, the pattern for the message is ``[A-Z0-9].+[^.!?,\s]``
