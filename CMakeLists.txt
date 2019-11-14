cmake_minimum_required(VERSION 3.10)
project(test_task2)

file(GLOB headers_hpp ${PROJECT_SOURCE_DIR}/headers/*.hpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/headers/*.h)
file(GLOB sources ${PROJECT_SOURCE_DIR}/sources/*.cpp)
include_directories(${PROJECT_SOURCE_DIR}/headers)


add_executable(${PROJECT_NAME} ${headers} ${sources} ${headers_hpp})

target_link_libraries(${PROJECT_NAME} pthread)

set_target_properties(${PROJECT_NAME} PROPERTIES
            CXX_STANDARD 11
            CXX_EXTENSIONS ON
            )
set(PROJECT_LIB ${PROJECT_NAME}_lib)

#add_subdirectory(test)
