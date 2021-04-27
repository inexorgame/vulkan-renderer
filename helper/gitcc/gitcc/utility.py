import re

from git import Repo, Commit


class RxSummary:
    rx_parser: re.Pattern = re.compile(r"\[(.*)] (.*)")
    rx_category: re.Pattern = re.compile(r"\*|(?:[a-z0-9]{2,}[\s|-]?)+")
    rx_description: re.Pattern = re.compile(r"[A-Z0-9].+[^.!?,\s]")

    def __init__(self, summary: str):
        self.category_tag: str = ""
        self.description_text: str = ""

        match = self.rx_parser.fullmatch(summary)
        if match is None:
            return
        self.category_tag = match.group(1)
        self.description_text = match.group(2)

    def valid_format(self) -> bool:
        return self.category_tag != "" and self.description_text != ""

    def valid_category_tag(self) -> bool:
        return self.rx_category.fullmatch(self.category_tag)

    def valid_description(self) -> bool:
        return self.rx_description.fullmatch(self.description_text)


def check_summary(summary: str) -> str:
    check = RxSummary(summary)
    if not check.valid_format():
        return "Invalid format. It should be '[<tag>] <Good Description>'"

    if not check.valid_category_tag():
        return "Invalid category tag. It should be either a single '*' or completely lowercase " \
               "letters or numbers, at least 2 characters long, other allowed characters are: '|', '-' and spaces."

    if not check.valid_description():
        return "Invalid description. It should start with an uppercase letter or number, " \
               "should be not to short and should not end with a punctuation."

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
