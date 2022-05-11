project(Colourotron VERSION 0.2.0)

cmake_minimum_required(VERSION 3.5)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

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

find_package(QT NAMES Qt6 COMPONENTS Widgets Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Concurrent REQUIRED)
find_package(OpenMP)
find_package(OpenCV REQUIRED core imgproc)
#set(TS_FILES macbeth-detect_fr_FR.ts)

qt_add_resources(resource_RCC resource.qrc theme/theme.qrc)

set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    imagemodel.cpp
    imagemodel.h
    graphicsview.cpp
    graphicsview.h
    graphicsscene.cpp
    graphicsscene.h    

    fittingdialog.cpp
    fittingdialog.h
    fittingdialog.ui

    macbethmodel.cpp
    macbethmodel.h

    macbethreferencemodel.cpp
    macbethreferencemodel.h

    macbethmeasuredmodel.cpp
    macbethmeasuredmodel.h

    macbethview.cpp
    macbethview.h

    radiometry.h

    resource.rc
    ${resource_RCC}
    #${TS_FILES}
    )

if (WIN32)
    add_executable(Colourotron WIN32 ${SOURCES})
    add_definitions(-DWIN32NOCONSOLE)
elseif (CMAKE_HOST_APPLE)
    add_executable(Colourotron MACOSX_BUNDLE ${SOURCES})
    set_target_properties(Colourotron PROPERTIES MACOSX_BUNDLE ON )

else()
    add_executable(Colourotron ${SOURCES})
endif()



if (OpenMP_FOUND OR OpenMP_CXX_FOUND)
   target_link_libraries(Colourotron PRIVATE OpenMP::OpenMP_CXX)
endif()

target_link_libraries(Colourotron PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Concurrent ${OpenCV_LIBS} image colors levmar)
# Eigen3::Eigen)

#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})


#-------------------------------------------------------------------------------
# INSTALL RULES FOR MacOS
#-------------------------------------------------------------------------------

# From ManaoExplorer
if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
    #qt5_add_resources(RESOURCE_FILES resource.qrc)
    # use, i.e. don't skip the full RPATH for the build tree
    set(CMAKE_SKIP_BUILD_RPATH  FALSE)

    # when building, don't use the install RPATH already
    # (but later on when installing)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    # Bundle creation on Mac
    set(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
    set(MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}")
    set(MACOSX_BUNDLE_CONTENTS    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/" )
    set(MACOSX_BUNDLE_RESOURCES   "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/Resources")
    set(MACOSX_BUNDLE_ICON_FILE   "Colourotron.icns")
    set(MACOSX_BUNDLE_ICON        "${PROJECT_SOURCE_DIR}/icons/${MACOSX_BUNDLE_ICON_FILE}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${MACOSX_BUNDLE_RESOURCES})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MACOSX_BUNDLE_ICON} ${MACOSX_BUNDLE_RESOURCES})
endif()

# Installation
if (UNIX AND NOT APPLE)
    string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)
    set(BIN_INSTALL_DIR "bin")
    set(DOC_INSTALL_DIR "share/doc/${PROJECT_NAME_LOWERCASE}/")
else()
    set(BIN_INSTALL_DIR ".")
    set(DOC_INSTALL_DIR ".")
endif()


# THE INSTALL RULE IS NECESSARY.
# WITHOUT it CPack will not package correctly
install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_INSTALL_DIR} COMPONENT Libraries)

if (UNIX AND NOT APPLE)
    install(FILES colourotron.desktop     DESTINATION "share/applications")
    install(FILES icons/Colourotron.png   DESTINATION "share/pixmaps")
elseif (WIN32)
    # THIS STRATEGY is from BRDFEXplorer and uses WindeployQt
    set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)
    # INSTALL WINDOWS DLL
    install(
        FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
        DESTINATION ${BIN_INSTALL_DIR}
        COMPONENT Libraries)

    include(Windeployqt)
    windeployqt(${PROJECT_NAME} ${BIN_INSTALL_DIR})
elseif (APPLE)
    include(Macdeployqt)
    macdeployqt(${PROJECT_NAME})
endif()

# CPack
include(ColourotronCPack)
