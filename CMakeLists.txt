# Minimum CMake version required, we'll just use the latest version.
cmake_minimum_required(VERSION 3.22)
# Project name, version and description
project(discord-bot VERSION 1.0 DESCRIPTION "A discord bot")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Create an executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/token.cpp
)

set(NLOHMANN_JSON_DIR ${CMAKE_SOURCE_DIR}/include)

# Find the mpg123 package
find_package(PkgConfig REQUIRED)
pkg_check_modules(MPG123 REQUIRED libmpg123)

# Find the mpg123 package
set(MPG123_INCLUDE_DIR "/usr/include")
set(MPG123_LIBRARY "/usr/lib/x86_64-linux-gnu/libmpg123.so")

link_directories(${MPG123_LIBRARY})

# Include directories
include_directories(${NLOHMANN_JSON_DIR} ${MPG123_INCLUDE_DIR})

# Find our pre-installed DPP package (using FindDPP.cmake).
# find_package(DPP REQUIRED)

# Find DPP package
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(DPP_ROOT_DIR "/usr/local")
find_package(DPP REQUIRED)

# Link the pre-installed DPP package.
target_link_libraries(${PROJECT_NAME}
    ${DPP_LIBRARIES}
    ${MPG123_LIBRARIES}
)

# Include the DPP directories.
target_include_directories(${PROJECT_NAME} PRIVATE
	${DPP_INCLUDE_DIRS}
    ${MPG123_LIBRARIES}
)

# Set C++ version
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
