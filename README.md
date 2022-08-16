# Imperial College: RealTime3D UI Application
###### Created by Nicholas Arnold, Philippa Mason and Jianguo Liu 

Project developed for the MOD and DSTL research call

----
## Setup
It is **_strongly_** recommended to use JetBrains CLion IDE with this project.
Free licenses are available with an `.ac.uk` email.

This project uses CMake for generating builds.

### Prerequisites
* Ensure [winget](https://docs.microsoft.com/en-us/windows/package-manager/winget/) package manager is installed
* Download [MSYS2](https://www.msys2.org/#installation) and follow instructions to install
* Ensure Python3.8 (Non Windows Store version) is installed
  * Installed at `C:\msys64\mingw64\bin\python.exe` on the HP ZBook
  * Otherwise:
      ```powershell 
      # to install python 3.8 using winget
      winget install -e --id Python.Python.3 -v 3.8.10150.0
      ```
* Install [Qt version 5.15.2](https://www.qt.io/blog/qt-5.15.2-released)
  * Installed at `C:/Qt/5.15.2/` on the HP ZBook

* Create python environment
  * Through CLion IDE UI:
    * File > Settings > Build, Execution, Deployment > Python Interpreter
    * Add interpreter > add local interpreter
    * Virtualenv Environment >
      * Environment: New
      * Location: C:/path/to/`RealTime3D/.venv/rt3d`
      * Base interpreter: `C:\msys64\mingw64\bin\python.exe`
    * open requirements.txt in the IDE
      * click 'Install requirements' in the notification bar at the top of the file
  * Manually:
      ```powershell
      # make sure to change directory to RealTime3D project directory
      # before runnign these commands
      python -m venv .\.venv\rt3d
      pip install -r requirements.txt
      ```
### Build and run the project
#### Through CLion IDE UI:
  * Setup MSYS2 toolchain (only needed once)
    * File > Settings > Build, Execution, Deployment > Toolchains
    * Add > MinGW
      * Name: MSYS2
      * Toolset: `C:\msys64\mingw64` 
      * (IDE should auto complete the rest)
  * Setup CMake (only needed once)
    * File > Settings > Build, Execution, Deployment > CMake
    * If Release and Debug profiles are not added, create them with the `+` and change the build types accordingly.
    * Add under CMake Options for all profiles:
        ```
        -DQt5_DIR=C:/Qt/5.15.2/mingw81_64/lib/cmake/Qt5
        -DPython3_ROOT_DIR=.venv/rt3d/Scripts/
        -DCMAKE_PREFIX_PATH=.\extern\lensfun\install
        -DCMAKE_INSTALL_PREFIX=..\dist
        ```
  * Choose `rt3d | release` or `rt3d | debug` and click the hammer icon to build
  * Click the play button to run the program
#### Using build scripts:
Update the paths in the desired script and run it
  * Release: `build_release.ps1`
  * Debug: `build_debug.ps1`
  * run `.\cmake-build-[release,debug]/rt3d.exe`

## To Install the project
#### Using CLion IDE UI
* Select: Build > Install

#### Using install scripts:
Update the paths in the desired script and run it
* Release: `install_release.ps1`
* Debug: `install_debug.ps1`

The installed application will be in `dist/rt3d_VERSION`

---- 
## Project layout
- scripts `location for python scripts`
- data `location of example data and database files`
  - dem_images `example data for generating DEMs`
  - navigation_images `example data for testing the Waypointer navigation`
  - perspective_images `example data for testing Warp Manager`
  - aircraftDB `aircraft profile files`
  - lensfunDB `lens and camera profile files`
- src `location of c++ source code`
  - dem_generation `the DEM processing`
  - device_profiles `source for the difference device settings`
    - aircraft
    - camera
    - lens
  - flight_parameters `source for calculating various paramters`
  - frame_shift_viewer `source for the frame shift visualisation module`
  - main_window `the main UI window`
  - navigation `source code for the Waypoint module`
  - settings `source for the applications settings`
  - utility `various utilites such as the Watchdog module, launching processes, and calling python scripts`
    - wia `windows image acquisition header files`
- include `<Not currently used> location for header files exposed for public consumption. `
- libs `location for library files`
- apps `location of applications that can be built`
- extern `location of external source code used by the project`
- cmake `location for CMake helper scripts`

---
### Converting Adobe profiles to lensfunDB files
Adobe lens correction profiles are stored in `data/lensfunDB/adobe`. These files are not uploaded to GitHub. Copies can be found on the HP ZBook or from the developers.

Use `python .\scripts/lensfun_convert_lcp.py -h` for information on how to use the program to convert these.

Place the .xml files created in the `data/lensfunDB/db` folder.

###### Note: The Lensfun main database lives in the `data/lensfunDB/db` folder.
