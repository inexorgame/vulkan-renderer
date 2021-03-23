import re
from dataclasses import dataclass

from git import Repo, Commit


@dataclass
class Check:
    rx: re.Pattern
    error_msg: str


SUMMARY = [
    Check(re.compile(r"\["), "Category description has no opening bracket '['."),
    Check(re.compile(r"[a-z0-9]"), "Category text should start with a lower case letter or number."),
    Check(re.compile(r"[a-z0-9\s|-]*[a-z0-9]"),
          "Category text has to be at least 2 characters long and can contain only the following characters:"
          " 'a-z', '0-9', '|', '-' and spaces."),
    Check(re.compile(r"]"),
          "Category text does not end with a lower case letter or category description has no closing bracket."),
    Check(re.compile(r"\s"), "No space after the category description."),
    Check(re.compile(r"[A-Z0-9]"), "Summary should start with a capital letter or number."),
    Check(re.compile(r".+?\s.+(?=.)"), "Summary is too short or not meaningful."),
    Check(re.compile(r"[^.!?,]"), "Summary should not end with a punctuation."),
]


def check_summary(summary: str) -> str:
    start_pos = 0
    for check in SUMMARY:
        match = check.rx.match(summary, start_pos)
        if match is None or match.start() != start_pos:
            return check.error_msg
        start_pos = match.end()
    return ""


def check_commit(commit: Commit) -> (bool, str):
    msg = check_summary(commit.summary)
    if msg == "":
        return True, f"Correct | {commit.hexsha} - {commit.summary}"
    else:
        return False, f"Failure | {commit.hexsha} - {commit.summary}\n" \
                      f"    Summary: {msg}"


def check_history(repo: Repo, exit_sha: str = "", include_correct: bool = False) -> (bool, list[str]):
    success = True
    msgs = []
    for commit in repo.iter_commits():
        if commit.hexsha == exit_sha:
            break
        res, msg = check_commit(commit)
        if not res or include_correct:
            msgs.append(msg)
        success = success and res
    return success, msgs


def check_branch(repo: Repo, source_branch: str, target_branch: str, include_correct: bool = False) -> (
        bool, list[str]):
    error_msgs = []

    common_ancestors = repo.merge_base(source_branch, target_branch)
    if len(common_ancestors) == 0:
        error_msgs.append("ERROR: No common ancestor found")
        return error_msgs

    start_commit = common_ancestors[0]
    return check_history(repo, exit_sha=start_commit, include_correct=include_correct)
