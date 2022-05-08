add_executable(extract-matrix main.c)
    
target_link_libraries(extract-matrix PRIVATE levmar colors)
