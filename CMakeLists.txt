cmake_minimum_required(VERSION 3.24)
project(ecs)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
link_directories(lib)
set(SFML_STATIC_LIBRARIES TRUE)
add_definitions(-DSFML_STATIC)
find_package(SFML COMPONENTS graphics window system REQUIRED)

add_executable(ecs src/main.cpp src/ECS.h src/MathUtils.h)

target_link_libraries(ecs sfml-graphics sfml-system sfml-window)

set_target_properties(ecs
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
        )