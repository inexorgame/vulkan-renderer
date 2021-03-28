import sys
from argparse import ArgumentParser

from git import Repo, InvalidGitRepositoryError
from gitcc.utility import check_branch, check_history, check_summary, check_commit


def cmd_parser():
    parser = ArgumentParser(prog="gitcc", description="")

    subparsers = parser.add_subparsers(dest='command', required=True)

    parser_summary = subparsers.add_parser('summary', help='check the given summary text')
    parser_summary.add_argument(dest='summary_text', metavar='text', help="text to check")

    parser_commit = subparsers.add_parser('commit', help='check current commit')
    parser_commit.add_argument(dest='repository', type=str, help='path to the repository')

    parser_history = subparsers.add_parser('history', help='check the current branch history')
    parser_history.add_argument(dest='repository', type=str, help='path to the repository')
    parser_history.add_argument('--sha', dest='sha', type=str, default="", help='check until this commit (exclusive)')
    parser_history.add_argument('--verbose', dest='verbose', action='store_true', help='print correct commits too')

    parser_branch = subparsers.add_parser('branch',
                                          help='check the current branch with an other branch common ancestor.'
                                               'Is the same as gitcc history with git merge-base <source> <target>')
    parser_branch.add_argument(dest='target_branch', type=str, metavar='target', help='target branch')
    parser_branch.add_argument(dest='repository', type=str, help='path to the repository')
    parser_branch.add_argument('--verbose', dest='verbose', action='store_true', help='print correct commits too')

    return parser


def main():
    args = cmd_parser().parse_args(sys.argv[1:])

    if args.command == 'summary':
        msg = check_summary(args.summary_text)
        if msg != "":
            print(msg)
            exit(1)
        else:
            print("Commit summary has the correct format!")
            exit(0)

    repo = None
    success = False
    try:
        repo = Repo(args.repository)
    except InvalidGitRepositoryError:
        print("Given path is not a repository.")
        exit(2)

    if args.command == 'commit':
        success, msg = check_commit(repo.head.commit)
        print(msg)
    else:
        msgs = []
        if args.command == 'history':
            success, msgs = check_history(repo, args.sha, include_correct=args.verbose)
        elif args.command == 'branch':
            success, msgs = check_branch(repo, repo.head, args.target_branch, include_correct=args.verbose)
        else:
            print("Invalid command")
            exit(1)
        if success and len(msgs) == 0:
            print("All commits have the correct format!")
        else:
            print('\n'.join(msgs))

    if success:
        exit(0)
    else:
        exit(1)
