cmake_minimum_required(VERSION 3.13)
project(osp-gen-images)
set(CMAKE_CXX_STANDARD 14)

find_package(rkcommon REQUIRED)
find_package(ospray 2.0.0 REQUIRED)
find_package(VTK REQUIRED)


add_library(cameras ArcballCamera.cpp
                    ArcballCamera.h)

target_link_libraries(cameras rkcommon::rkcommon)

add_library(params_reader ParamReader.cpp
                          ParamReader.h)       

add_executable(gen_images_vtk gen_image_vtk.cpp)
set_target_properties(gen_images_vtk PROPERTIES
                                  CXX_STANDARD 14
                                  CXX_STANDARD_REQUIRED ON)  
target_link_libraries(gen_images_vtk PUBLIC ospray::ospray 
                                            rkcommon::rkcommon
                                            cameras
                                            params_reader) 
target_compile_definitions(gen_images_vtk PUBLIC -DOSPRAY_CPP_RKCOMMON_TYPES)  

target_include_directories(gen_images_vtk PUBLIC ${VTK_INCLUDE_DIRS})

target_link_libraries(gen_images_vtk PUBLIC ${VTK_LIBRARIES})

# add_executable(get_range get_range.cpp)
# set_target_properties(get_range PROPERTIES
#                                   CXX_STANDARD 14
#                                   CXX_STANDARD_REQUIRED ON)  
# target_link_libraries(get_range PUBLIC ospray::ospray 
#                                        rkcommon::rkcommon) 
# target_compile_definitions(get_range PUBLIC
#                                       -DOSPRAY_CPP_RKCOMMON_TYPES)  

# add_executable(gen_images gen_images.cpp)
# set_target_properties(gen_images PROPERTIES
#                                   CXX_STANDARD 14
#                                   CXX_STANDARD_REQUIRED ON)  
# target_link_libraries(gen_images PUBLIC ospray::ospray 
#                                        rkcommon::rkcommon
#                                        cameras) 
# target_compile_definitions(gen_images PUBLIC
#                                       -DOSPRAY_CPP_RKCOMMON_TYPES)  

# target_include_directories(gen_images PUBLIC ${VTK_INCLUDE_DIRS})

# target_link_libraries(gen_images PUBLIC ${VTK_LIBRARIES})

# add_executable(find_cameras find_cameras.cpp)
# set_target_properties(find_cameras PROPERTIES
#                                   CXX_STANDARD 14
#                                   CXX_STANDARD_REQUIRED ON)  
# target_link_libraries(find_cameras PUBLIC ospray::ospray 
#                                        rkcommon::rkcommon) 
# target_compile_definitions(find_cameras PUBLIC
#                                       -DOSPRAY_CPP_RKCOMMON_TYPES)  


