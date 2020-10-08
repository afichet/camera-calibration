find_package(TIFF REQUIRED)

add_executable(extract-patches main.c)

include_directories(extract-patches ${TIFF_INCLUDE_DIR})
target_link_libraries(extract-patches PUBLIC colors ${TIFF_LIBRARY})
