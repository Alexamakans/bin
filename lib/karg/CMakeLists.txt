# Build static library: karg

file(GLOB_RECURSE SRC_FILES "*.c")
add_library(karg STATIC ${SRC_FILES})

# Make headers accessible to projects that link this library
target_include_directories(karg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
