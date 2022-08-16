import sys
from pathlib import Path
from importlib.util import find_spec
import pkg_resources
import subprocess


def get_module_info(package):
    result = subprocess.run([sys.executable, "-m", "pip", "show", package], capture_output=True, text=True)
    result_text = result.stdout
    result_dict = {p[:p.find(':')]: p[p.find(':') + 2:]
                   for p in result_text.splitlines(False)}
    return result_dict


def retrieve_package_dirs(package_name):
    # get package info
    package_info_ = get_module_info(package_name)
    # get package location
    package_loc = Path(package_info_['Location'].strip())
    package_dirs = [d for d in package_loc.glob(f'*{package_name}*')]
    if not package_dirs:
        package_name = package_name.replace('-', '_')
        package_dirs = [d for d in package_loc.glob(f'*{package_name}*')]
    for d in package_dirs:
        if d.suffix == '.dist-info':
            # check that top level name is the same as the package
            with (d / "top_level.txt").open('r') as f:
                top_level_name = f.readline().strip()
            top_level_loc = package_loc / top_level_name
            # if it has already been found: continue the function
            if top_level_loc in package_dirs:
                break
            # otherwise, add it to the dirs
            package_dirs.append(top_level_loc)
            break
    package_dirs = [p.as_posix() for p in package_dirs]
    cmake_list_ = ';'.join(package_dirs)
    return cmake_list_, package_info_


def check_requires(package_dict):
    # check requires and collect if any
    if not package_info['Requires']:
        return

    required_packages_dir = [retrieve_package_dirs(p.strip())[0] for p in package_info['Requires'].split(',')]
    cmake_list_ = ';'.join(required_packages_dir)
    return cmake_list_


if __name__ == '__main__':
    # get package name from command line
    query_package = sys.argv[1]
    cmake_list, package_info = retrieve_package_dirs(query_package)

    required_cmake_list = check_requires(package_info)
    if required_cmake_list is not None:
        cmake_list = cmake_list + ";" + required_cmake_list

    print(cmake_list, end='')
