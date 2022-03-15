cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE SRC "main.cpp" "src/*.cpp" "src/*.hpp" "glad/src/*.c")

include_directories("src")
include_directories("stb_image")
include_directories("SFML-2.5.1/include")
include_directories("glm")
include_directories("glad/include")

# add the executable
add_executable(boids ${SRC})

# set the project name
project(Flocking)

# link_directories(SFML-2.5.1/lib)
target_link_directories(boids PUBLIC SFML-2.5.1/lib)
target_link_libraries(boids sfml-graphics sfml-window sfml-system sfml-audio)

add_custom_command(
    TARGET boids POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/SFML-2.5.1/bin/sfml-audio-2.dll" "${CMAKE_SOURCE_DIR}/build/sfml-audio-2.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/SFML-2.5.1/bin/sfml-graphics-2.dll" "${CMAKE_SOURCE_DIR}/build/sfml-graphics-2.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/SFML-2.5.1/bin/sfml-window-2.dll" "${CMAKE_SOURCE_DIR}/build/sfml-window-2.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/SFML-2.5.1/bin/sfml-system-2.dll" "${CMAKE_SOURCE_DIR}/build/sfml-system-2.dll"
)