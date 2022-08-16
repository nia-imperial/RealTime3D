# ------ FILL IN ------
$project_path = "C:\Users\arnoldn\CLionProjects\RealTime3D"
# path where cmake executable lives
$cmake_path = "C:\Users\arnoldn\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\222.3345.126\bin\cmake\win\bin\cmake.exe"

# ------ Leave Alone ------
& $cmake_path `
--build "$project_path\cmake-build-debug" `
--target install `
-j 12