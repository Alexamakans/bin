# Build static library: libpasswd


file(GLOB_RECURSE SRC_FILES "*.c")

add_library(libpasswd STATIC ${SRC_FILES})

# Make headers accessible to projects that link this library
target_include_directories(libpasswd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
