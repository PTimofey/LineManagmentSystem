cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(Line_Management_System VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(Line_Management_System Line_Managment_System.cpp)

# Include directories
include_directories(${PROJECT_SOURCE_DIR})

# Add external libraries if needed
# e.g., linking with pthreads
find_package(Threads REQUIRED)
target_link_libraries(Line_Management_System PRIVATE Threads::Threads)

# Additional flags (optional)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

# Set the output directory
set_target_properties(Line_Management_System PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
)
