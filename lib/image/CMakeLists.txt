find_package(TIFF)

set(PUBLIC_HEADERS
    image.h
    )

add_library(image STATIC
    image.cpp
    )

if (TIFF_FOUND)
    include_directories(image ${TIFF_INCLUDE_DIR})
    add_definitions(-DHAS_TIFF)
    target_link_libraries(image PUBLIC tiff)
endif()

set_target_properties(image PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
target_include_directories(image PUBLIC include)

include_directories(image ../../external/tinyexr)

if (MSVC)
    target_compile_options(image PUBLIC /W3)
else()
    target_link_libraries(image PUBLIC m)
    target_compile_options(image PUBLIC -Wall -Wextra -Wpedantic)
endif()


target_link_libraries(image PUBLIC colors lodepng)

install(TARGETS image
    ARCHIVE DESTINATION ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
