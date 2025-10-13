import sys
from pathlib import Path

import sphinx_rtd_theme

sys.path.insert(0, str(Path('../../').resolve()))
sys.path.insert(0, str(Path('../').resolve()))

from helper import generate_radar
# generate additional source stuff
generate_radar.generate(Path("./auto-generated/"))

extensions = [
    'breathe',
    'exhale',
    'sphinxcontrib.mermaid'
]
source_suffix = ['.rst']
master_doc = 'index'

project = 'Inexor vulkan-renderer'
author = 'Inexor Collective'
copyright = '2020-present ' + author + '. The page content is licensed under CC ' \
 + 'BY 4.0 unless otherwise noted'
title = project + ' Documentation'
version = 'v0.1-alpha.3'
release = 'v0.1-alpha.3'

language = 'english'
pygments_style = 'sphinx'
html_theme = 'sphinx_rtd_theme'
html_logo = 'images/inexor.png'
html_theme_options = {
    'logo_only': False,
    'display_version': True,
}
html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]
html_static_path = ['_static']
html_css_files = [
    'css/style.css',
]
html_js_files = [
    'js/svg-highlight.js',
]
html_favicon = "images/inexor_48x48px.ico"

linkcheck_ignore = [
    r"https://github.com/.*#"  # do not check anchors from GitHub, JS magic
]

mermaid_version = "11.4.1"

# Setup the breathe extension
breathe_projects = {
    "inexor-vulkan-renderer": "../doxygen-output/xml"
}
breathe_default_project = "inexor-vulkan-renderer"
breathe_default_members = ('private-members', 'members', 'undoc-members')

# Setup the exhale extension
header_path = "../../include/"
exhale_args = {
    # These arguments are required
    "containmentFolder": "./exhale-generated",
    "rootFileName": "main.rst",
    "rootFileTitle": "C++ Code Documentation",
    "doxygenStripFromPath": header_path,
    # Suggested optional arguments
    "createTreeView": True,
    # TIP: if using the sphinx-bootstrap-theme, you need
    # "treeViewIsBootstrap": True,
    "exhaleExecutesDoxygen": True,
    "exhaleDoxygenStdin": f"""INPUT = {header_path}
    EXTRACT_PRIVATE = YES
    EXTRACT_ALL = YES""",
    "verboseBuild": True,
}

# Tell sphinx what the primary language being documented is.
primary_domain = 'cpp'
# Tell sphinx what the pygments highlight language should be.
highlight_language = 'cpp'
