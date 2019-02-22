cmake_minimum_required(VERSION 3.13)
project(simplelocker)

set(CMAKE_CXX_STANDARD 17)


add_subdirectory(boost-cmake)

FUNCTION(add_sdbus_subdir)
    set(ENABLE_TESTS OFF CACHE BOOL "Disable sdbus-cpp tests")
    add_subdirectory(sdbus-cpp/sdbus-cpp sdbus-cpp/build)
ENDFUNCTION ()

add_sdbus_subdir()

get_filename_component(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" ABSOLUTE BASE_DIR "${CMAKE_BINARY_DIR}")

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib64")

include_directories(${CMAKE_SOURCE_DIR}/sdbus-cpp/sdbus-cpp/include)

add_executable(simplelocker main.cpp Inhibitor.h DbusSignalListener.h DbusService.h XIdleChecker.h XIdleChecker.cpp)
add_dependencies(simplelocker sdbus-c++)
target_link_libraries(simplelocker PUBLIC sdbus-c++)
target_link_libraries(simplelocker PUBLIC Boost::program_options)

target_compile_options(simplelocker PRIVATE -Wall)
target_link_options(simplelocker PRIVATE -lpthread -lXss -lX11)

install(TARGETS simplelocker sdbus-c++
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

