set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION}${VERSION_FLAG})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "CNRS and INRIA")
set(CPACK_PACKAGE_DESCRIPTION ${PROJECT_NAME})

if(APPLE)
    set(CPACK_BUNDLE_NAME ${PROJECT_NAME})
    set(CPACK_BUNDLE_PLIST "${MACOSX_BUNDLE_CONTENTS}/Info.plist")
    set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/icons/${MACOSX_BUNDLE_ICON_FILE}")
    set(CPACK_BUNDLE_ICON "${CPACK_PACKAGE_ICON}")
endif()

set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} ${PROJECT_NAME})
set(CPACK_CREATE_DESKTOP_LINKS ${PROJECT_NAME})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")

configure_file("${CMAKE_CURRENT_LIST_DIR}/ColourotronCPackOptions.cmake.in"
    "${Colourotron_BINARY_DIR}/ColourotronCPackOptions.cmake" @ONLY)
set(CPACK_PROJECT_CONFIG_FILE
    "${Colourotron_BINARY_DIR}/ColourotronCPackOptions.cmake")

include(CPack)
