cmake_minimum_required(VERSION 3.5)

#project(macbeth-detect LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#find_package(Eigen3 3 REQUIRED)
#if (EIGEN3_FOUND)
#    message("-- Eigen 3 found !")
#else()
#    message("-- Eigen 3 NOT found !")
#endif()

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

#find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
#find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Concurrent REQUIRED)
find_package(OpenMP)
find_package(OpenCV REQUIRED core imgproc)
#set(TS_FILES macbeth-detect_fr_FR.ts)

add_executable(gui
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    macbethmodel.cpp
    macbethmodel.h
    graphicsview.cpp
    graphicsview.h
    graphicsscene.cpp
    graphicsscene.h
    theme/theme.qrc

    #${TS_FILES}
)


if (OpenMP_FOUND OR OpenMP_CXX_FOUND)
   target_link_libraries(gui PRIVATE OpenMP::OpenMP_CXX)
endif()

target_link_libraries(gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Concurrent ${OpenCV_LIBS} image colors)
# Eigen3::Eigen)

#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})


