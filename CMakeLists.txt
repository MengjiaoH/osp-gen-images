cmake_minimum_required(VERSION 3.13)
project(osp-gen-images)
set(CMAKE_CXX_STANDARD 14)

find_package(rkcommon REQUIRED)
find_package(ospray 2.0 REQUIRED)

add_executable(gen_images main.cpp)
set_target_properties(gen_images PROPERTIES
                                  CXX_STANDARD 14
                                  CXX_STANDARD_REQUIRED ON)  
target_link_libraries(gen_images PUBLIC ospray::ospray
                                        rkcommon::rkcommon) 
target_compile_definitions(gen_images PUBLIC
                                      -DOSPRAY_CPP_RKCOMMON_TYPES)  
   