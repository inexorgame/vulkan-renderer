from pathlib import Path

import pkg_resources
import sys
from argparse import ArgumentParser


def cmd_parser():
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
                print(f"{pkg} was NOT found on your system, but is required.")
        except pkg_resources.VersionConflict as ex:
            success = False
            if not args.quiet:
                print(f"{ex.dist} was found on your system, but {ex.req} is required.")
        else:
            if not args.quiet:
                print(f"{pkg} was found on your system.")
    exit(0) if success else exit(1)


if __name__ == '__main__':
    main()
