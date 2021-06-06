"""
Copyright 2021-present Iceflower - iceflower@gmx.de

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

from argparse import ArgumentParser
from pathlib import Path

import pkg_resources
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

    success = True
    for pkg in dependencies.split('\n'):
        try:
            pkg_resources.require(pkg)
        except pkg_resources.DistributionNotFound:
            success = False
            if not args.quiet:
                print(f"Did not found '{pkg}', but is required.")
        except pkg_resources.VersionConflict as ex:
            success = False
            if not args.quiet:
                print(f"Found: '{ex.dist}', but '{ex.req}' is required.")
        else:
            if not args.quiet:
                print(f"Found: '{pkg}'.")
    exit(0) if success else exit(1)


if __name__ == '__main__':
    main()
