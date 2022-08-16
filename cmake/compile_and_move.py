import sys
from pathlib import Path
import py_compile

package = Path(sys.argv[1]).expanduser().resolve()
dest = Path(sys.argv[2]).expanduser().resolve()
print(package)
print(dest)

if not package.exists():
    quit(-1)

if not dest.exists():
    print("making compiled scripts folder")
    dest.mkdir()

pyc_name = package.with_suffix('.pyc')
py_compile.compile(package.as_posix(), (dest/pyc_name.name).as_posix())


