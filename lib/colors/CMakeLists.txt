set(PUBLIC_HEADERS
    color-convert.h
    spectrum-converter.h
    io.h
    macbeth-data.h
    )

add_library(colors STATIC
    color-converter.c
    spectrum-converter.c
    io.c
    )

set_target_properties(colors PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
target_include_directories(colors PUBLIC include)

if (MSVC)
    target_compile_options(colors PUBLIC /W3)
else()
    target_link_libraries(colors PUBLIC m)
    target_compile_options(colors PUBLIC -Wall -Wextra -Wpedantic)
endif()

install(TARGETS colors
    ARCHIVE DESTINATION ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
