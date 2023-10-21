cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(libWarehouseSim)

find_package(Boost 1.58 REQUIRED)
find_package(yaml-cpp REQUIRED)

#include_directories(
#  include
#  ${Boost_INCLUDE_DIRS}
#  #../libHARMONIES/externalDependencies/aws-sdk-cpp/aws-cpp-sdk-core/include
#  ../externalDependencies/rpclib/include
#  #../libHARMONIES/include
#  ../warehouse_gazebo_plugin/include
#)
#
#add_library(libWarehouseSim
#	 # src/warehouseSimHarmonies.cpp
#  src/warehouseSimGazebo.cpp
#)
#target_link_libraries(libWarehouseSim
#	#aws-cpp-sdk-core
#  #libHarmonies
#  ${YAML_CPP_LIBRARIES}
#  rpc
#  pthread
#)

#add_subdirectory(examples)
