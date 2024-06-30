cmake_minimum_required(VERSION 3.10)

# Project name
project(secuencial)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add executable
add_executable(secuencial v4.cpp boleta1.cpp bd.cpp pbasic.cpp)

# Find SQLite3 package
find_package(SQLite3 REQUIRED)

# Include directories
target_include_directories(secuencial PRIVATE ${SQLite3_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR})

# Link libraries
target_link_libraries(secuencial PRIVATE ${SQLite3_LIBRARIES})

# Find OpenMP package
find_package(OpenMP REQUIRED)

# Add OpenMP flags to the compiler
if(OpenMP_CXX_FOUND)
    target_compile_options(secuencial PRIVATE ${OpenMP_CXX_FLAGS})
    target_link_libraries(secuencial PRIVATE OpenMP::OpenMP_CXX)
endif()
