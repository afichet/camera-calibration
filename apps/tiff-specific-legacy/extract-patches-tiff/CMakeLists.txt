find_package(TIFF REQUIRED)

add_executable(extract-patches main.c)

include_directories(extract-patches-tiff ${TIFF_INCLUDE_DIR})
target_link_libraries(extract-patches-tiff PUBLIC colors ${TIFF_LIBRARY})
