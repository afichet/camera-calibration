find_package(TIFF)

add_executable(correct-image-tiff main.c)

if (TIFF_FOUND)
    include_directories(correct-image-tiff ${TIFF_INCLUDE_DIR})
    add_definitions(-DHAS_TIFF)
endif()

target_link_libraries(correct-image-tiff PUBLIC colors ${TIFF_LIBRARIES})
