set(project_name device)

# BEGIN sel4 dependencies
include(../../settings.cmake)

cmake_minimum_required(VERSION 3.7.2)

project(${project_name} C ASM)

find_package(seL4 REQUIRED)
find_package(elfloader-tool REQUIRED)
find_package(musllibc REQUIRED)
find_package(util_libs REQUIRED)
find_package(seL4_libs REQUIRED)
find_package(projects_libs REQUIRED)

sel4_import_kernel()
elfloader_import_project()

# This sets up environment build flags and imports musllibc and runtime libraries.
musllibc_setup_build_environment_with_sel4runtime()
sel4_import_libsel4()
util_libs_import_libraries()
sel4_libs_import_libraries()
# END sel4 dependencies
projects_libs_import_libraries()

add_subdirectory(../libsel4tutorials libsel4tutorials)
add_subdirectory(../os_case os_case)

# BEGIN custom
add_executable(${project_name} main.c)
target_link_libraries(${project_name} sel4muslcsys muslc sel4runtime sel4tutorials os_case usbdrivers pci)
# END custom

include(rootserver)
DeclareRootserver(${project_name})

include(simulation)
GenerateSimulateScript()
