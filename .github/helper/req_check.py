"""
Check if all packages of a python requirement file are satisfied.

Copyright 2021-present Iceflower - iceflower@gmx.de

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

from argparse import ArgumentParser
from pathlib import Path

import importlib.metadata as im
import packaging.version as pv
import re
import sys


def cmd_parser() -> ArgumentParser:
    parser = ArgumentParser(prog="req_check",
                            description="Check if a python installation matches a requirements.txt file")

    parser.add_argument(dest='req_file', type=str, help='path to requirements.txt')
    parser.add_argument('--quiet', dest='quiet', action='store_true', default=False, help='output nothing')

    return parser


def main():
    args = cmd_parser().parse_args(sys.argv[1:])

    with Path(args.req_file).open(mode='r', encoding='UTF-8') as reader:
        dependencies = reader.read()

    success: bool = True
    rx_pkg: re.Pattern = re.compile(r"(.+)(==|<=|>=)(.+)")
    for pkg in dependencies.split('\n'):
        match: re.Match = rx_pkg.match(pkg)
        pkg_name: str = match.group(1)
        pkg_req: str = match.group(2)
        pkg_version: pv.Version = pv.parse(match.group(3))
        try:
            installed_version: pv.Version = pv.parse(im.version(pkg_name))
            if (
                pkg_req == "==" and installed_version != pkg_version
                or pkg_req == "<=" and installed_version > pkg_version
                or pkg_req == ">=" and installed_version < pkg_version
            ):
                success = False
                if not args.quiet:
                    print(f"Found: '{installed_version}', but '{pkg_req}{pkg_version}' is required.")
            elif not args.quiet:
                print(f"Found: '{pkg}'.")
        except im.PackageNotFoundError:
            success = False
            if not args.quiet:
                print(f"Did not found '{pkg}', but is required.")
    exit(0) if success else exit(1)


if __name__ == '__main__':
    main()
