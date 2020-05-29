import sys
from pathlib import Path

import sphinx_rtd_theme

sys.path.insert(0, str(Path('../../').resolve()))
extensions = [
    'breathe',
    'exhale',
    'recommonmark',
    'sphinxcontrib.mermaid',
]
source_suffix = ['.rst']
master_doc = 'index'

project = 'Inexor Vulkan Renderer'
author = 'Inexor'
copyright = 'CC BY 2020 Inexor'
title = project + ' Documentation'
version = '0.0.0'
release = '0.0.0'

language = 'english'
pygments_style = 'sphinx'
html_theme = 'sphinx_rtd_theme'
html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]
html_static_path = ['_static']
html_css_files = [
    'css/style.css',
]
html_js_files = [
    'js/svg-highlight.js',
]
html_favicon = "../../assets/textures/logo_rendered.png"

# Setup the breathe extension
breathe_projects = {
    "vulkan-renderer": "../doxygen-output/xml"
}
breathe_default_project = "vulkan-renderer"
breathe_default_members = ('private-members', 'members', 'undoc-members')

# Setup the exhale extension
header_path = "../../include/"
exhale_args = {
    # These arguments are required
    "containmentFolder": "./exhale-generated",
    "rootFileName": "main.rst",
    "rootFileTitle": "Source Code",
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
