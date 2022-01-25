#!/usr/bin/env python

import sys
from pathlib import Path

from gitcc import utility

if __name__ == '__main__':
    commit_msg_file = Path(sys.argv[1])
    with commit_msg_file.open('r') as reader:
        commit_summary = reader.read().split('\n')[0]
    res = utility.check_summary(commit_summary)
    if res == "":
        exit(0)
    print(res)
    exit(1)
