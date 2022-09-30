"""
Will generate an python intersphinx inventory file based on the Vulkan Specification.
This generator will ALWAYS use `enum` as role otherwise some linking is broken.
It might link to NOT existing webpages, if the type has not an own page in the man directory (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/)
Expected working directory is ./documentation.
"""
import sys
from pathlib import Path
import xml.etree.ElementTree as ET

import certifi
import re

import sphobjinv as soi
import urllib3


def get_vk_xml(vk_version: list[str]) -> ET.ElementTree:
    """
    Get the VK API xml.
    """
    with urllib3.PoolManager(cert_reqs='CERT_REQUIRED', ca_certs=certifi.where()) as https:
        with https.request('GET', f"https://github.com/KhronosGroup/Vulkan-Docs/blob/v{'.'.join(vk_version[:3])}/xml/vk.xml",
                           preload_content=False, retries=urllib3.util.retry.Retry(3)) as req:
            return ET.parse(req.data)


def get_vulkan_version(conanfile: Path) -> list[str]:
    """
    Get the current Vulkan version used by this project.
    [MAJOR, MINOR, PATCH]
    """
    sys.path.append(str(conanfile.parent.resolve()))
    from conanfile import InexorConan
    rx_vk = re.compile(r'vulkan-headers/(\d+\.\d+\.\d+\.\d+)')
    for req in InexorConan.requires:
        if match := rx_vk.fullmatch(req):
            return match.group(1).split('.')
    raise RuntimeError("Vulkan version not found")


gen_names: set[str] = set()


def check_exist(name):
    """
    Will print names which are already added, this should not happen.
    """
    global gen_names
    if name in gen_names:
        print(f"WARNING: {name} already added!")
    else:
        gen_names.add(name)


def create_data_obj_str(role: str, name: str, url: str) -> soi.DataObjStr:
    """
    Create data object strings to be added to the inventory.
    """
    # TODO: Comment out this line while developing.
    # check_exist(name)
    return soi.DataObjStr(name=name, domain="cpp", role=role, priority='1', uri=url, dispname='-')


def create_inventory(tree: ET.ElementTree, vk_version: list[str]) -> soi.Inventory:
    """
    Create the inventory.
    """
    inv: soi.inventory = soi.Inventory()
    inv.project = "Vulkan"
    inv.version = '.'.join(vk_version)

    # types
    def get_name(el: ET.Element) -> str:
        name = el.attrib.get("name", None)
        if name is None:
            return el.find("name").text
        return name

    for typ in tree.findall("/types/type"):
        role: str = ""
        match category := typ.attrib.get("category", None):
            case "basetype":
                for test_role in [("struct", "struct"), ("typedef", "type")]:
                    if test_role[0] in typ.text:
                        role = test_role[1]
                if role == "":
                    print(f"Unknown role {role}")
                    continue
            case "bitmask":
                role = "type"
            case "define":
                role = "macro"
            case "enum":
                role = "enum"
            case "funcpointer":
                role = "type"
            case "handle":
                role = "type"
            case "struct":
                role = "struct"
            case "union":
                role = "union"
            case None | "include":
                continue
            case _:
                print(f"Unknown category {category}")
                print(typ.attrib)
                continue
        name: str = get_name(typ)
        page: str = ""
        if alias := typ.attrib.get("alias", None) is None:
            page = name
        else:
            page = alias
        # TODO: use the real role instead of enum
        # workaround that some names are not getting linked
        inv.objects.append(create_data_obj_str("enum", name, f"{page}.html"))

        # add struct member
        if role == "struct":
            for member in typ.findall("member"):
                inv.objects.append(create_data_obj_str("member", f"{name}.{member.find('name').text}", f"{page}.html"))
    # constants
    for enums in tree.find("/enums/[@name='API Constants']").iter():
        name: str = enums.attrib['name']
        inv.objects.append(create_data_obj_str('var', name, f"{name}.html"))
    # enums
    for enums in tree.findall("/enums/[@name!='API Constants']"):
        page: str = enums.attrib['name']
        for enum in enums.findall("enum"):
            inv.objects.append(create_data_obj_str('enumerator', enum.attrib['name'], f"{page}.html"))
    # functions
    for func in tree.findall("/commands/command"):
        page: str
        name: str
        if "alias" in func.attrib:
            name = func.attrib['name']
            page = func.attrib['alias']
        else:
            name = func.find("proto/name").text
            page = name
        inv.objects.append(create_data_obj_str('func', name, f"{page}.html"))

    return inv


def gen_intersphinx_inventory(vk_version: list[str], output_file: Path, skip_check: bool = False):
    """
    Generate the Vulkan intersphinx inventory.
    :param vk_version: Vulkan specification, might get it with get_vulkan_version
    :param output_file: output path of the inventory file
    :param skip_check: skip checking if the inventory file is up to date and force a recreation
    """
    if not skip_check:
        try:
            if soi.Inventory(output_file).version == '.'.join(vk_version):
                print("Vulkan inventory file is up to date. Nothing to do.")
                return
        except TypeError:
            pass

    print(f"Generating Vulkan {vk_version} inventory.")
    tree: ET.ElementTree = get_vk_xml(vk_version)
    inv: soi.Inventory = create_inventory(tree, vk_version)

    soi.writebytes(output_file, soi.compress(inv.data_file(contract=False)))
