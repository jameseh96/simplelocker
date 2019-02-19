cmake_minimum_required(VERSION 3.13)
project(simplelocker)

set(CMAKE_CXX_STANDARD 17)


add_subdirectory(boost-cmake)

include(ExternalProject)

get_filename_component(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" ABSOLUTE BASE_DIR "${CMAKE_BINARY_DIR}")

ExternalProject_Add(EP_sdbus-c++
        PREFIX ${CMAKE_SOURCE_DIR}/sdbus-cpp
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/sdbus-cpp/src
        STAMP_DIR ${CMAKE_SOURCE_DIR}/sdbus-cpp/stamp
        BINARY_DIR ${CMAKE_SOURCE_DIR}/sdbus-cpp/build
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        CMAKE_ARGS -DENABLE_TESTS=OFF -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
        BUILD_ALWAYS 1
        STEP_TARGETS build
        )

set(SDBUS_CPP_LIB_DIR "${CMAKE_SOURCE_DIR}/sdbus-cpp/install/lib64")

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib64")
set(SDBUS_CPP_LIBRARIES
    "${CMAKE_INSTALL_RPATH}/${CMAKE_SHARED_MODULE_PREFIX}sdbus-c++${CMAKE_SHARED_LIBRARY_SUFFIX}"
)

include_directories(${CMAKE_SOURCE_DIR}/sdbus-cpp/src/include)

add_executable(simplelocker main.cpp Inhibitor.h DbusSignalListener.h DbusService.h XIdleChecker.h XIdleChecker.cpp)
add_dependencies(simplelocker EP_sdbus-c++)
target_link_libraries(simplelocker PUBLIC ${SDBUS_CPP_LIBRARIES})
target_link_libraries(simplelocker PUBLIC Boost::program_options)

target_compile_options(simplelocker PRIVATE -Wall)
target_link_options(simplelocker PRIVATE -lpthread -lXss -lX11)

install(TARGETS simplelocker
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)


