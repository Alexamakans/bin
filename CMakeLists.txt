cmake_minimum_required(VERSION 3.31)

# Set the project name and C language
project(bin C)

# Use compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C standard globally (no GNU extensions)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF) # Ensures -std=c99, not -std=gnu99

# Add POSIX compliance flag
add_compile_definitions(_POSIX_C_SOURCE=200809L)

# Enable extra warnings to help enforce portability
add_compile_options(-Wall -Wextra -Wpedantic)

# Optionally detect unsupported platforms and warn
if(WIN32)
  message(WARNING "POSIX compliance may not be guaranteed on Windows")
endif()

# Add all libraries in lib/
file(
  GLOB LIB_DIRS
  RELATIVE ${CMAKE_SOURCE_DIR}/lib
  ${CMAKE_SOURCE_DIR}/lib/*)
foreach(lib ${LIB_DIRS})
  if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${lib})
    add_subdirectory(lib/${lib})
  endif()
endforeach()

# Add all projects in src/
file(
  GLOB PROJECT_DIRS
  RELATIVE ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/*)
foreach(proj ${PROJECT_DIRS})
  if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/src/${proj})
    add_subdirectory(src/${proj})
  endif()
endforeach()
