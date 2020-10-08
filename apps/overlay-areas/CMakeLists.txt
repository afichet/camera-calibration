find_package(TIFF REQUIRED)

add_executable(overlay-areas main.c)

include_directories(overlay-areas ${TIFF_INCLUDE_DIR})
target_link_libraries(overlay-areas PUBLIC colors ${TIFF_LIBRARY})
