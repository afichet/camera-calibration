find_package(TIFF REQUIRED)

add_executable(correct-image main.c)

include_directories(correct-image ${TIFF_INCLUDE_DIR})
target_link_libraries(correct-image PUBLIC colors ${TIFF_LIBRARY})
