from importlib import resources
from pathlib import Path

mod_path: str = "gitcc.git_hooks"


def get_res_path(mod_path: str, file_name: str) -> Path:
    with resources.path(mod_path, file_name) as p:
        return Path(p)


COMMIT_MSG_SUMMARY = get_res_path(mod_path, 'commit_msg_summary.py')
