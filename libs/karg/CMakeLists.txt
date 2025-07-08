# Build static library: karg

add_library(karg STATIC
    karg.c
)

# Make headers accessible to projects that link this library
target_include_directories(karg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
