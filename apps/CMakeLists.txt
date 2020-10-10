add_subdirectory(gen-colorchart-image)
add_subdirectory(gen-ref-colorchart)
add_subdirectory(extract-patches)
add_subdirectory(overlay-areas)

add_subdirectory(extract-matrix)
add_subdirectory(correct-patches)
add_subdirectory(correct-image)

find_package(QT QUIET)

if (QT_FOUND)
  add_subdirectory(gui)
endif()