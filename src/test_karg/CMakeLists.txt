# Build executable: test_karg

add_executable(test_karg
    main.c
)

# === Linking custom libraries (uncomment and modify as needed) ===
target_link_libraries(test_karg PRIVATE karg)

# === If you need include directories (rare in simple apps) ===
# target_include_directories(test_karg PRIVATE ${CMAKE_SOURCE_DIR}/libs/libA)
