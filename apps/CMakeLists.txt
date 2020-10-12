add_subdirectory(derawzinator)
add_subdirectory(demo-demoz)

add_subdirectory(gen-colorchart-image)
add_subdirectory(gen-ref-colorchart)
add_subdirectory(extract-patches)
add_subdirectory(overlay-areas)

add_subdirectory(extract-matrix)
add_subdirectory(correct-patches)
add_subdirectory(correct-image)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets QUIET)

if (QT_VERSION_MAJOR)
  add_subdirectory(gui)
endif()
