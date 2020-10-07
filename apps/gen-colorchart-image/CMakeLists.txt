add_executable(gen-colorchart-image main.c)
    
target_link_libraries(gen-colorchart-image PUBLIC colors lodepng)
