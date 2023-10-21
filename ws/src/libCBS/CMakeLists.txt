cmake_minimum_required(VERSION 3.5)

project(libCBS)

# Gather all source files in the src/ directory
file(GLOB SOURCES "src/*.cpp")
# Gather all header files in the include/ directory
file(GLOB HEADERS "include/libCBS/*.h")

include_directories(
        include/libCBS
)

# Define your library target
add_library(libCBS STATIC
        ${SOURCES}
)