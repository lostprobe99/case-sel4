
cmake_minimum_required(VERSION 3.8.2)

add_library(os_case STATIC EXCLUDE_FROM_ALL src/sel4thread.c)

target_link_libraries(
    os_case 
    sel4_autoconf
    sel4runtime
    muslc
    sel4
    sel4platsupport
    sel4muslcsys
    sel4runtime_Config
    sel4tutorials
)

# We force a dependency on the constructor symbol otherwise the linker won't link in the file
target_link_libraries(os_case -Wl,-u -Wl,register_debug_putchar)
target_include_directories(os_case PUBLIC include)