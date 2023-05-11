import sys
from pathlib import Path

import sphinx_rtd_theme

sys.path.insert(0, str(Path('../../').resolve()))
sys.path.insert(0, str(Path('../').resolve()))

from helper import generate_radar, generate_vk_inv

# generate additional source stuff
vk_version = generate_vk_inv.get_vulkan_version(Path("../../conanfile.py"))
generate_radar.generate(Path("./auto-generated/"))
generate_vk_inv.gen_intersphinx_inventory(vk_version, Path("./vulkan_objects.inv"))

extensions = [
    'breathe',
    'exhale',
    'recommonmark',
    'sphinxcontrib.mermaid',
    'sphinx.ext.intersphinx',
    'sphinx.ext.todo',
]
source_suffix = ['.rst']
master_doc = 'index'

project = 'Inexor Vulkan Renderer'
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
html_extra_path = [
    "vulkan_objects.inv",
]
html_favicon = "../../assets/textures/logo_rendered.png"

intersphinx_mapping = {
    'vulkan': (f"https://registry.khronos.org/vulkan/specs/{'.'.join(vk_version[:2])}-extensions/man/html/", 'vulkan_objects.inv'),
}

linkcheck_ignore = [
    r"https://github.com/.*#"  # do not check anchors from GitHub, JS magic
]

mermaid_version = "8.9.2"

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
    EXTRACT_ALL = YES
    ALIASES  = "rst=\\verbatim embed:rst:leading-asterisk"
    ALIASES += "endrst=\\endverbatim"
""",
    "verboseBuild": True,
}

# Tell sphinx what the primary language being documented is.
primary_domain = 'cpp'
# Tell sphinx what the pygments highlight language should be.
highlight_language = 'cpp'
