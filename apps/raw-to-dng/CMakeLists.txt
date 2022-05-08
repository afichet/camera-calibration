add_executable(raw-to-dng main.c matrix3x3.c)
target_link_libraries(raw-to-dng PRIVATE image colors)
