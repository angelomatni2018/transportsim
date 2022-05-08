cmake_minimum_required(VERSION 3.10)
project(TransportSim)

add_compile_options(-std=c++2a)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(build_transportsim_lib src_dir_name)
    file(GLOB SRC_CPP_FILES src/${src_dir_name}/*.cpp)
    list(FILTER SRC_CPP_FILES EXCLUDE REGEX ".*_test.cpp$")
    add_library(transportsim_${src_dir_name} SHARED ${SRC_CPP_FILES})
    target_include_directories(transportsim_${src_dir_name} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    install(TARGETS transportsim_${src_dir_name} LIBRARY DESTINATION lib)
endfunction()

build_transportsim_lib( base )
build_transportsim_lib( pathing )
build_transportsim_lib( simulator )
set(TRANSPORTSIM_LIBS "transportsim_base;transportsim_pathing;transportsim_simulator")

function(entrypoint_using_sfml bin_path bin_name)
    add_executable(${bin_name}_bin src/${bin_path}/${bin_name}.cpp)
    set_target_properties(${bin_name}_bin PROPERTIES OUTPUT_NAME ${bin_name})
    target_include_directories(${bin_name}_bin PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    target_link_libraries(${bin_name}_bin LINK_PUBLIC ${TRANSPORTSIM_LIBS} "sfml-audio;sfml-graphics;sfml-network;sfml-system;sfml-window")
    install(TARGETS ${bin_name}_bin RUNTIME DESTINATION bin)
endfunction()

entrypoint_using_sfml(simulator game_simulator)
entrypoint_using_sfml(pathing pathreconciler_test)
