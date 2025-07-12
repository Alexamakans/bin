# Build executable: test_karg

file(GLOB_RECURSE SRC_FILES "*.c")
add_executable(test_karg ${SRC_FILES})

# === Linking custom libraries (uncomment and modify as needed) ===
target_link_libraries(test_karg PRIVATE karg)

# === If you need include directories (rare in simple apps) ===
# target_include_directories(test_karg PRIVATE ${CMAKE_SOURCE_DIR}/lib/libA)
