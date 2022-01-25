import filecmp
import shutil
from pathlib import Path

from gitcc.git_hooks import COMMIT_MSG_SUMMARY


def install_summary_git_hook(project_dir: Path, force: bool = False):
    """
    :param force: override existing files
    :param project_dir: must contain a .git directory.
    :return:
    """
    hook_file = project_dir.joinpath(".git/hooks/commit-msg")
    if not force and hook_file.exists():
        raise FileExistsError("A commit message hook already exist!")
    shutil.copy2(COMMIT_MSG_SUMMARY, hook_file)


def uninstall_summary_git_hook(project_dir: Path, force: bool = False):
    """
    :param force: remove file even if it might not be an gitcc hook
    :param project_dir: must contain a .git directory.
    :return:
    """
    hook_file = project_dir.joinpath(".git/hooks/commit-msg")
    if not force and not filecmp.cmp(COMMIT_MSG_SUMMARY, hook_file, shallow=False):
        raise RuntimeError("A commit message hook was found, but it might be customized, not removing.")
    hook_file.unlink(missing_ok=True)


GIT_HOOKS = {
    'install': {
        'summary': install_summary_git_hook,
    },
    'uninstall': {
        'summary': uninstall_summary_git_hook,
    }
}


def cmd_git_hook(repo: Path, action: str, hooks: list[str], force: bool = False) -> int:
    """

    :param repo: repository path
    :param action: action to execute
    :param hooks: git hooks
    :param force: force action
    :return: exit code
    """
    exit_code = 0
    for hook in hooks:
        try:
            GIT_HOOKS[action][hook](repo, force)
        except (FileExistsError, RuntimeError) as ex:
            print(ex)
            exit_code = 1
    return exit_code
