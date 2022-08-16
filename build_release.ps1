# ------ FILL IN ------
$project_path = "C:\Users\arnoldn\CLionProjects\RealTime3D"

# path where cmake executable lives
$cmake_path = "C:\Users\arnoldn\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.3345.126\bin\cmake\win\bin\cmake.exe"
# path where ninja make program lives
$make_path = "C:/Users/arnoldn/AppData/Local/JetBrains/Toolbox/apps/CLion/ch-0/222.3345.126/bin/ninja/win/ninja.exe"

# path to Qt5 cmake scripts lives
$qt5_cmake_dir = "C:/Qt/5.15.2/mingw81_64/lib/cmake/Qt5"
# Python interpreter environment lives
$py_interp = "$project_path\.venv/rt3d/Scripts/"
# Location of lensfun install
$lensfun_install = "$project_path\extern\lensfun\install"


# ------ Leave Alone ------
& $cmake_path `
-DCMAKE_BUILD_TYPE=Release `
-DCMAKE_MAKE_PROGRAM="$make_path" `
-DQt5_DIR="$qt5_cmake_dir" `
-DPython3_ROOT_DIR="$py_interp" `
-DCMAKE_PREFIX_PATH="$lensfun_install" `
-G Ninja `
-S "$project_path" `
-B "$project_path\cmake-build-release" `
-DCMAKE_INSTALL_PREFIX="$project_path/dist" `
