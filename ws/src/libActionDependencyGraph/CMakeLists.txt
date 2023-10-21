cmake_minimum_required(VERSION 3.5)

project(libActionDependencyGraph)

include_directories(
  include
)

add_executable(testADG
  src/test.cpp
)

set_property(TARGET testADG PROPERTY CXX_STANDARD 11)
