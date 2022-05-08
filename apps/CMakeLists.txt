add_subdirectory(derawzinator)
add_subdirectory(demo-demoz)

add_subdirectory(gen-colorchart-image)
add_subdirectory(gen-ref-colorchart)
add_subdirectory(extract-patches)
add_subdirectory(overlay-areas)

add_subdirectory(extract-matrix)
add_subdirectory(correct-patches)
add_subdirectory(correct-image)

add_subdirectory(advanced-fit)
add_subdirectory(raw-to-dng)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets QUIET)
find_package(OpenCV COMPONENTS core imgproc QUIET)

if (QT_VERSION_MAJOR AND OpenCV_FOUND)
  add_subdirectory(gui)
else()
  message("The GUI Colourotron will not be built: you need Qt & OpenCV")
endif()
