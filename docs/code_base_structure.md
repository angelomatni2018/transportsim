# CMake Build

This small project has chosen to stick with 1 root CMakeLists.txt and a flat directory structure. To have your libraries and executables automatically found and compiled, follow these conventions:
* All source must be contained under `src/MODULE_NAME/*.cpp` and `include/MODULE_NAME/*.h`
* Your module must be added to the `SUBDIRS` variable in CMakeLists.txt in order of dependencies (1st module cannot have dependencies, 2nd module can only depend on the 1st, etc...)
    * TODO: Provide DAG of dependencies instead of single list to cmake to optimize library compilation. See GH-17
* All entrypoints should be in `*_main.cpp` files
* All tests should be in `*_test.cpp` files

# CI/CD

This project has yet to compose a docker image for use in CI/CD as it has minimal dependencies. As such, a base ubuntu image is used and dependencies are installed every time (see `.github/workflows` and `dependencies.sh`)