set(PUBLIC_HEADERS
    lodepng.h
    )

add_library(lodepng STATIC
    lodepng.c
    )

set_target_properties(lodepng PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
target_include_directories(lodepng PUBLIC .)


if (MSVC)
    target_compile_options(lodepng PUBLIC /W3)
else()
    target_compile_options(lodepng PUBLIC -Wall -Wextra -Wpedantic)
endif()

install(TARGETS lodepng
    ARCHIVE DESTINATION ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
