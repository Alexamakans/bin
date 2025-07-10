# Build executable: ls

file(GLOB_RECURSE SRC_FILES "*.c")

add_executable(ls ${SRC_FILES})

# === Linking custom libraries (uncomment and modify as needed) ===
target_link_libraries(
  ls
  PRIVATE karg
          libpasswd
          # -lm for math.h
          -lm)

# === If you need include directories (rare in simple apps) ===
# target_include_directories(ls PRIVATE ${CMAKE_SOURCE_DIR}/libs/libA)
