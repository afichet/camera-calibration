find_package(TIFF)

set(PUBLIC_HEADERS
    include/image.h
    include/imagergb.h
    include/imageraw.h
    include/imageprocessing.h
    include/demosaicamaze.h
    )

add_library(image STATIC
    image.cpp
    imagergb.cpp
    imageraw.cpp
    imageprocessing.cpp
    demosaicamaze.cpp
    )

if (TIFF_FOUND)
    include_directories(image ${TIFF_INCLUDE_DIR})
    add_definitions(-DHAS_TIFF)

    target_link_libraries(image PUBLIC TIFF::TIFF)
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


find_package(OpenMP)

if (OpenMP_FOUND OR OpenMP_CXX_FOUND)
   target_link_libraries(image PUBLIC OpenMP::OpenMP_CXX)
endif()


target_link_libraries(image PUBLIC colors tinyxml2)

#install(TARGETS image
#    ARCHIVE DESTINATION ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
#    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#)
