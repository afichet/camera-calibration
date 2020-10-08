find_package(TIFF)

add_executable(correct-image main.c)

if (TIFF_FOUND)
    include_directories(correct-image ${TIFF_INCLUDE_DIR})
    add_definitions(-DHAS_TIFF)
endif()

include_directories(correct-image ../../external/tinyexr)
target_link_libraries(correct-image PUBLIC colors ${TIFF_LIBRARIES})
