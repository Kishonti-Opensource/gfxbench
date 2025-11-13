cmake_minimum_required(VERSION 3.5)

cmake_policy(SET CMP0020 NEW)
include(CheckRequiredVars)
check_required_vars(PRODUCT_ID PRODUCT_VERSION PRODUCT_NAME)

# Set compiler flags to support C++11 features
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(VersionParse)
version_parse(BENCHMARK ${PRODUCT_VERSION})

# Backward compatibility with AppStore package names
if(APPLE)

    string(REPLACE "_" "." PACKAGE_NAME "net.kishonti.${PRODUCT_ID}.osx")

    if("${PRODUCT_ID}" STREQUAL "gfxbench_gl")

        set(PACKAGE_NAME "net.kishonti.osx.GFXBench")
    elseif("${PRODUCT_ID}" STREQUAL "compubench_cl")

        set(PACKAGE_NAME "net.kishonti.osx.CompuBench")
    else()

        set(PACKAGE_NAME "${PACKAGE_NAME}")
    endif()

else()

    string(REPLACE "_" "." PACKAGE_NAME "net.kishonti.${PRODUCT_ID}")
endif()

project(${PRODUCT_ID})

if(DEFINED ENV{CCACHE})
  include(ccache.cmake)
endif()

string(TOUPPER ${PRODUCT_ID} PRODUCT_ID_UPPERCASE)
add_definitions(-D${PRODUCT_ID_UPPERCASE})


if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DELAYLOAD:vulkan-1.dll")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DELAYLOAD:d3d12.dll")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DELAYLOAD:d3dcompiler_47.dll")
endif()

set(NG_TARGET_PLATFORM "$ENV{NG_TARGET_PLATFORM}" CACHE STRING "store platform")

if (DEFINED BENCHMARK_DIR)
    add_subdirectory(${BENCHMARK_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT_ID})
endif()

if(NOT TARGET benchmarkservice)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../frameworks/benchmark_service ${CMAKE_CURRENT_BINARY_DIR}/benchmark_service)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/resources/${PRODUCT_ID}/variables.cmake")
set(RESOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/resources/${PRODUCT_ID})
if (NOT EXISTS ${RESOURCE_DIR})
    set(RESOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/resources/compubench_cl)
endif()

include("${RESOURCE_DIR}/variables.cmake")
include(apply_find_vars)
include(compiler_flags)
include(make_qrc.cmake)

if (${CXX_COMPILER_ID_UPPER} STREQUAL "CLANG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-warning-option -Wno-inconsistent-missing-override")
endif()

find_package(ngrtl REQUIRED core)
apply_find_vars(NGRTL)

# Add source files

configure_file(${CMAKE_CURRENT_LIST_DIR}/src/PresentationLayer/config.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/config.cpp)
list(APPEND SOURCE ${CMAKE_CURRENT_BINARY_DIR}/config.cpp)

include(${CMAKE_CURRENT_LIST_DIR}/src/Widgets/Widgets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/src/PresentationLayer/PresentationLayer.cmake)

set(TESTFW_ROOT ${CMAKE_CURRENT_LIST_DIR}/../frameworks/testfw)
list(APPEND EXT_HEADERS
    ${TESTFW_ROOT}/hostapp/qt/glwidget.h
    ${TESTFW_ROOT}/graphics/qgraphicscontext.h
)
list(APPEND EXT_SOURCE
    ${TESTFW_ROOT}/hostapp/qt/glwidget.cpp
    ${TESTFW_ROOT}/graphics/qgraphicscontext.cpp
)

if(WIN32)
    list(APPEND EXT_HEADERS ${TESTFW_ROOT}/graphics/dxwin32graphicscontext.h)
    list(APPEND EXT_SOURCE ${TESTFW_ROOT}/graphics/dxwin32graphicscontext.cpp)
    list(APPEND EXT_SOURCE ${TESTFW_ROOT}/graphics/vulkangraphicscontext.cpp)
    list(APPEND SOURCE ${RESOURCE_DIR}/forms/resources_win.rc)
endif()

if (APPLE AND NOT IOS AND NOT XCODE_VERSION VERSION_LESS 7.0)
    list(APPEND EXT_HEADERS ${TESTFW_ROOT}/graphics/metalgraphicscontext.h)
    list(APPEND EXT_SOURCE ${TESTFW_ROOT}/graphics/metalgraphicscontext.mm)
    set_source_files_properties(src/PresentationLayer/BenchmarkPage.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set_source_files_properties(src/PresentationLayer/BenchmarkPage.cpp PROPERTIES COMPILE_DEFINITIONS HAS_METAL)
    list(APPEND LIBRARIES "-weak_framework Metal")
endif()

list(APPEND MOC_FILES ${TESTFW_ROOT}/hostapp/qt/glwidget.h)

make_qrc(${CMAKE_CURRENT_BINARY_DIR}/localization.qrc "" ${TFW_PACKAGE_DIR}/localization)
list(APPEND RC_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/localization.qrc
    ${RESOURCE_DIR}/forms/resources.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/${PRODUCT_ID}/forms/resources.qrc
)

if(OPT_COMMUNITY_BUILD)
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/src/PresentationLayer/main.cpp PROPERTIES COMPILE_DEFINITIONS IS_COMMUNITY)
endif()

if(NOT EXISTS ${QT5_BIN_PATH})
    unset(QT5_BIN_PATH)
    unset(QT5_BIN_PATH CACHE)
endif()
set(QT5_BIN_PATH "$ENV{QT5_BIN_PATH}" CACHE STRING "store binary platform of QT")

# Use Qt
if(QT5_BIN_PATH STREQUAL "")
    find_program(QMAKE qmake HINTS $ENV{QMAKE})
else()
    if(NOT QMAKE)
        if(EXISTS ${QT5_BIN_PATH}/qmake)
            set(QMAKE "${QT5_BIN_PATH}/qmake" CACHE STRING "Store bin path" FORCE)
        elseif(EXISTS ${QT5_BIN_PATH}/qmake.exe)
            set(QMAKE "${QT5_BIN_PATH}/qmake.exe" CACHE STRING "Store bin path" FORCE)
        else()
            message(FATAL_ERROR "qmake didn't exists. check QT path")
        endif()
    endif()
endif()
message(STATUS "QMAKE=${QMAKE}")

execute_process(COMMAND ${QMAKE} -query QT_INSTALL_PLUGINS OUTPUT_VARIABLE QT_PLUG_TMP_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
#execute_process(COMMAND ${QMAKE} -query QT_INSTALL_HEADERS OUTPUT_VARIABLE QT_HEADERS_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${QMAKE} -query QT_INSTALL_BINS OUTPUT_VARIABLE QT_BINARY_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
#execute_process(COMMAND ${QMAKE} -query QT_VERSION OUTPUT_VARIABLE QT_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)

message("QT_INSTALL_PLUGINS =${QT_PLUG_TMP_DIR}")
message("QT_INSTALL_LIBS =${QT_LIBRARY_DIR}")
message("QT_INSTALL_HEADERS =${QT_HEADERS_DIR}")
message("QT_INSTALL_BINS =${QT_BINARY_DIR}")
message("QT_VERSION =${QT_VERSION}")


if(NOT EXISTS ${QT_PLUGINS_DIR})
  unset(QT_PLUGINS_DIR)
  unset(QT_PLUGINS_DIR CACHE)
endif()

if(DEFINED ENV{QT_PLUGINS_DIR})
  set(QT_PLUG_TMP_DIR $ENV{QT_PLUGINS_DIR})
endif()

if(EXISTS ${QT_PLUG_TMP_DIR})
  set(QT_PLUGINS_DIR ${QT_PLUG_TMP_DIR} CACHE STRING "Store plugins folder")
endif()

if(NOT EXISTS ${QT_PLUGINS_DIR})
    message(FATAL_ERROR "Set up a valid QT_PLUGINS_DIR folder! current:${QT_PLUGINS_DIR} ")
endif()

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_LIBRARY_DIR}/cmake)

if(NG_TARGET_PLATFORM MATCHES "vs20[0-9][0-9]-arm64")
    set(QT_PLUGINS_DIR "$ENV{QT5_ARM64_PATH}/plugins"  CACHE STRING "Store plugins folder" FORCE)
    if (NOT TARGET Qt5::qmake)
        add_executable(Qt5::qmake IMPORTED)
        set_target_properties(Qt5::qmake PROPERTIES IMPORTED_LOCATION "${QT5_BIN_PATH}/qmake.exe")
    endif()

    if (NOT TARGET Qt5::moc)
        add_executable(Qt5::moc IMPORTED)
        set_target_properties(Qt5::moc PROPERTIES IMPORTED_LOCATION "${QT5_BIN_PATH}/moc.exe")
    endif()

    if (NOT TARGET Qt5::uic)
        add_executable(Qt5::uic IMPORTED)
        set_target_properties(Qt5::uic PROPERTIES IMPORTED_LOCATION "${QT5_BIN_PATH}/uic.exe")
    endif()

    if (NOT TARGET Qt5::rcc)
        add_executable(Qt5::rcc IMPORTED)
        set_target_properties(Qt5::rcc PROPERTIES IMPORTED_LOCATION "${QT5_BIN_PATH}/rcc.exe")
    endif()
endif()

if(DEFINED ENV{QT5_ARM64_PATH} AND NG_TARGET_PLATFORM MATCHES "vs20[0-9][0-9]-arm64")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{QT5_ARM64_PATH})
    set(QT_LIBRARY_DIR $ENV{QT5_ARM64_PATH}/lib)
    set(QT_BINARY_DIR $ENV{QT5_ARM64_PATH}/bin)
else()
    execute_process(COMMAND ${QMAKE} -query QT_INSTALL_LIBS OUTPUT_VARIABLE QT_LIBRARY_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_LIBRARY_DIR}/cmake)
endif()

set(QT_MODULES
    Core
    Gui
    Widgets
    OpenGL
    Xml
)
foreach(qt_module ${QT_MODULES})
    find_package("Qt5${qt_module}" REQUIRED)
endforeach()

qt5_wrap_ui(UIC ${UI_FILES})
qt5_add_resources(RCC ${RC_FILES})
qt5_wrap_cpp(MOC ${MOC_FILES})

source_group("Source" FILES ${HEADERS} ${SOURCE})
source_group("Source\\Generated\\MOC" FILES ${MOC})
source_group("Source\\Generated\\UIC" FILES ${UIC})
source_group("Source\\Generated\\RCC" FILES ${RCC})
source_group("Source\\External" FILES ${EXT_HEADERS} ${EXT_SOURCE})

# Add targets

if(UNIX AND NOT APPLE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH true)
endif()

if(APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${HEADERS} ${SOURCE} ${EXT_HEADERS} ${EXT_SOURCE} ${MOC} ${UIC} ${RCC})
elseif(WIN32 AND OPT_COMMUNITY_BUILD)
    add_executable(${PROJECT_NAME} WIN32 ${HEADERS} ${SOURCE} ${EXT_HEADERS} ${EXT_SOURCE} ${MOC} ${UIC} ${RCC})
else()
    add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCE} ${EXT_HEADERS} ${EXT_SOURCE} ${MOC} ${UIC} ${RCC})
endif()


if(MSVC)
    target_link_libraries(${PROJECT_NAME} "advapi32.lib;gdi32.lib;shell32.lib")
endif()

foreach(module ${TESTFW_MODULES})
    add_dependencies(${PROJECT_NAME} ${module})
endforeach()

set_target_properties(${PROJECT_NAME} PROPERTIES
    XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
)

set(TFW_DEVELOPMENT_DIR ${CMAKE_BINARY_DIR}/tfw-dev)
# This need to be set for every build type to avoid creating Debug, Release, etc. subdirectories.
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${TFW_DEVELOPMENT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${TFW_DEVELOPMENT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${TFW_DEVELOPMENT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${TFW_DEVELOPMENT_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX _d)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${TESTFW_ROOT}
    ${TESTFW_ROOT}/hostapp/qt
    ${TESTFW_ROOT}/schemas
    ${CMAKE_CURRENT_LIST_DIR}/../frameworks/benchmark_service/src
    ${CMAKE_CURRENT_BINARY_DIR}/benchmark_service
)

list(APPEND LIBRARIES benchmarkservice ${TESTFW_LIBRARIES} ${APPLY_FIND_LIBRARIES})

if(UNIX AND NOT APPLE)
    list(APPEND LIBRARIES
        ${QT_LIBRARY_DIR}/libicuuc.so.56
        ${QT_LIBRARY_DIR}/libicudata.so.56
        ${QT_LIBRARY_DIR}/libicui18n.so.56)
endif()

if(APPLE)
    set_property(TARGET ${PROJECT_NAME} PROPERTY XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${PACKAGE_NAME})
    math(EXPR VERSION_CODE "${BENCHMARK_VERSION_MAJOR}*10000 + ${BENCHMARK_VERSION_MINOR}*100 + ${BENCHMARK_VERSION_PATCH}")
    set(BUNDLE_VERSION "${VERSION_CODE}.$ENV{BUILD_NUMBER}")
    configure_file(${CMAKE_CURRENT_LIST_DIR}/resources/MacOSXBundleInfo.plist.in ${CMAKE_BINARY_DIR}/Info.plist)
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_BINARY_DIR}/Info.plist)
    set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
    list(APPEND LIBRARIES
        "-framework AppKit"
        "-framework ApplicationServices"
        "-framework IOKit"
        "-framework SystemConfiguration"
        "-weak_framework QuartzCore"
    )
    if(XCODE)
        set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
    endif()
elseif(UNIX)
    list(APPEND LIBRARIES "${CMAKE_DL_LIBS}" )
endif()

if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${MSVC_INCLUDE_SYMBOLS} Psapi.lib WinInet.lib D3d9.lib")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"libcmt.lib\"")
endif()

target_link_libraries(${PROJECT_NAME}
    tfw_schemas
    tfw_deviceinfo
    ${LIBRARIES}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::Xml
)

# Installation

if(OPT_COMMUNITY_BUILD)
    set(CPACK_PACKAGE_FILE_NAME "${PRODUCT_ID}-${NG_TARGET_PLATFORM}-qt-${PRODUCT_VERSION}+community")
else()
    set(CPACK_PACKAGE_FILE_NAME "${PRODUCT_ID}-${NG_TARGET_PLATFORM}-qt-${PRODUCT_VERSION}+corporate")
endif()

if(WIN32)
    if("${OPT_CONFIG}" STREQUAL "Debug")
        file(COPY ${QT_PLUGINS_DIR}/platforms/qwindowsd.dll DESTINATION "${TFW_PACKAGE_DIR}/bin/platforms")
        file(COPY ${QT_PLUGINS_DIR}/imageformats/qjpegd.dll DESTINATION "${TFW_PACKAGE_DIR}/bin/imageformats")
    else()
        file(COPY ${QT_PLUGINS_DIR}/platforms/qwindows.dll DESTINATION "${TFW_PACKAGE_DIR}/bin/platforms")
        file(COPY ${QT_PLUGINS_DIR}/imageformats/qjpeg.dll DESTINATION "${TFW_PACKAGE_DIR}/bin/imageformats")
    endif()
    set(DIRS ${QT_BINARY_DIR} ${QT_LIBRARY_DIR} ${QT_PLUGINS_DIR} ${CMAKE_INSTALL_PREFIX}/lib ${CMAKE_BINARY_DIR}/benchmark_service/${OPT_CONFIG})
    set(BIN_DIR "$ENV{NG_BUILD_ROOT}/${NG_TARGET_PLATFORM}/app_qt/${OPT_CONFIG}")

    add_custom_target(fixup ALL DEPENDS ${PROJECT_NAME})
if(NG_TARGET_PLATFORM MATCHES "vs20[0-9][0-9]-arm64")
    file(COPY ${QT_PLUGINS_DIR}/../bin/libGLESv2.dll DESTINATION "${TFW_PACKAGE_DIR}/bin")
    file(COPY ${QT_PLUGINS_DIR}/../bin/Qt5Core.dll DESTINATION "${TFW_PACKAGE_DIR}/bin")
    file(COPY ${QT_PLUGINS_DIR}/../bin/Qt5Gui.dll DESTINATION "${TFW_PACKAGE_DIR}/bin")
    file(COPY ${QT_PLUGINS_DIR}/../bin/Qt5OpenGL.dll DESTINATION "${TFW_PACKAGE_DIR}/bin")
    file(COPY ${QT_PLUGINS_DIR}/../bin/Qt5Widgets.dll DESTINATION "${TFW_PACKAGE_DIR}/bin")
    file(COPY ${QT_PLUGINS_DIR}/../bin/Qt5Xml.dll DESTINATION "${TFW_PACKAGE_DIR}/bin")
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/arm64/14.29.30133/msvcp140.dll DESTINATION "${TFW_PACKAGE_DIR}/bin")
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/arm64/14.29.30133/msvcp140_1.dll DESTINATION "${TFW_PACKAGE_DIR}/bin")
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/arm64/14.29.30133/vcruntime140.dll DESTINATION "${TFW_PACKAGE_DIR}/bin")
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/arm64/14.29.30133/vcruntime140_1.dll DESTINATION "${TFW_PACKAGE_DIR}/bin")

    if(OPT_COMMUNITY_BUILD)
        add_custom_command(TARGET fixup POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/netman.dll ${TFW_PACKAGE_DIR}/bin
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/ngrtl_core.dll ${TFW_PACKAGE_DIR}/bin
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/syncdir.dll ${TFW_PACKAGE_DIR}/bin)
    endif()
    add_custom_command(TARGET fixup POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${TFW_PACKAGE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${TFW_PACKAGE_DIR}/bin $<TARGET_FILE_DIR:${PROJECT_NAME}>)

else()
    if(NG_TARGET_PLATFORM MATCHES "vs20[0-9][0-9]")
        add_custom_command(TARGET fixup POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/bin/amd_ags_x64.dll ${TFW_PACKAGE_DIR}/bin
        )
    endif()
    add_custom_command(TARGET fixup POST_BUILD
        COMMAND ${CMAKE_COMMAND} "-DSRC=$<TARGET_FILE:${PROJECT_NAME}>" "-DDST=${TFW_PACKAGE_DIR}/bin" "-DDIRS=${DIRS}" "-DOPT_CONFIG=${OPT_CONFIG}" -P ${CMAKE_CURRENT_LIST_DIR}/../frameworks/cmake-utils/cmake/fixup.cmake
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${TFW_PACKAGE_DIR}/bin $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

    if(NOT OPT_COMMUNITY_BUILD)
        install(DIRECTORY ${TFW_PACKAGE_DIR}/config DESTINATION ".")
        install(DIRECTORY ${TFW_PACKAGE_DIR}/data DESTINATION ".")
    endif()
    install(DIRECTORY ${TFW_PACKAGE_DIR}/bin DESTINATION ".")
    install(DIRECTORY ${TFW_PACKAGE_DIR}/plugins DESTINATION ".")
    install(DIRECTORY ${TFW_PACKAGE_DIR}/images DESTINATION ".")
    if(NOT NG_TARGET_PLATFORM MATCHES "vs20[0-9][0-9]-arm64")
        include(InstallRequiredSystemLibraries)
    endif()
endif()

if(UNIX AND NOT APPLE) # Linux
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        SKIP_BUILD_RPATH false
        BUILD_WITH_INSTALL_RPATH true)

    set(DIRS ${QT_BINARY_DIR} ${QT_LIBRARY_DIR} ${QT_PLUGINS_DIR} ${CMAKE_INSTALL_PREFIX}/lib)
    set(BIN_DIR "$ENV{NG_BUILD_ROOT}/${NG_TARGET_PLATFORM}/app_qt/${OPT_CONFIG}")

    add_custom_target(fixup ALL DEPENDS ${PROJECT_NAME})
    add_custom_command(TARGET fixup POST_BUILD
        COMMAND ${CMAKE_COMMAND} "-DSRC=$<TARGET_FILE:${PROJECT_NAME}>" "-DDST=${TFW_PACKAGE_DIR}/bin" "\"-DDIRS=${DIRS}\"" "-DOPT_CONFIG=${OPT_CONFIG}" -P ${CMAKE_CURRENT_LIST_DIR}/../frameworks/cmake-utils/cmake/fixup.cmake
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${TFW_PACKAGE_DIR}/bin $<TARGET_FILE_DIR:${PROJECT_NAME}>)

    if(OPT_COMMUNITY_BUILD)
        if ("${OPT_CONFIG}" STREQUAL Debug)
            get_target_property(POSTFIX ${PROJECT_NAME} DEBUG_POSTFIX)
        endif()
        configure_file(${CMAKE_CURRENT_LIST_DIR}/resources/application.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT_ID}.desktop)
        #install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT_ID}.desktop DESTINATION share/applications)
        #install(FILES ${RESOURCE_DIR}/images/icon.png DESTINATION lib/${PACKAGE_NAME}/images)
        install(FILES ${CMAKE_CURRENT_LIST_DIR}/resources/eula.txt DESTINATION "." RENAME copyright)
        #install(FILES ${CMAKE_CURRENT_LIST_DIR}/resources/changelog.gz DESTINATION share/doc/${PACKAGE_NAME})
        install(
            DIRECTORY ${TFW_PACKAGE_DIR}/bin/
            DESTINATION "."
            PATTERN "libQt*" EXCLUDE
            PATTERN "libicu*" EXCLUDE
            PATTERN "${PROJECT_NAME}" EXCLUDE)
        install(
            FILES ${TFW_PACKAGE_DIR}/bin/${PROJECT_NAME}
            DESTINATION "."
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        install(DIRECTORY ${TFW_PACKAGE_DIR}/plugins DESTINATION ".")
        install(DIRECTORY ${TFW_PACKAGE_DIR}/images DESTINATION ".")
        include(InstallRequiredSystemLibraries)
    else()
        file(COPY ${QT_PLUGINS_DIR}/platforms DESTINATION "${TFW_PACKAGE_DIR}/bin/")
        file(COPY ${QT_PLUGINS_DIR}/imageformats/libqjpeg.so DESTINATION "${TFW_PACKAGE_DIR}/bin/imageformats")
        install(DIRECTORY ${TFW_PACKAGE_DIR}/config DESTINATION ".")
        install(DIRECTORY ${TFW_PACKAGE_DIR}/data DESTINATION ".")
        install(DIRECTORY ${TFW_PACKAGE_DIR}/bin DESTINATION . USE_SOURCE_PERMISSIONS)
        install(DIRECTORY ${TFW_PACKAGE_DIR}/plugins DESTINATION . USE_SOURCE_PERMISSIONS)
        install(DIRECTORY ${TFW_PACKAGE_DIR}/images DESTINATION .)
    endif()
endif()

if(APPLE)
    # this code sign identity stored in keys repo OSX.keychain

    # To upload community version to appstore connect:
    # Ensure that it is signed with your "3rd Party Mac Developer Installer" certificate.
    #
    set(DEVELOPER_CODE_SIGN_IDENTITY "Apple Distribution: Kishonti")
    set(INSTALLER_CODE_SIGN_IDENTITY "3rd Party Mac Developer Installer: Kishonti")

    #
    # To create corporate package use Developer ID:
    #
    set(DEVELOPER_CODE_SIGN_IDENTITY "Developer ID Application: Kishonti")
    set(INSTALLER_CODE_SIGN_IDENTITY "Developer ID Installer: Kishonti")


    set(APP_BUNDLE ${PROJECT_NAME}.app)
    set(APP_CONTENTS_DIR ${APP_BUNDLE}/Contents)
    set(APP_PLUGINS_DIR ${APP_CONTENTS_DIR}/PlugIns)

    set(DIRS ${QT_LIBRARY_DIR} ${QT_PLUGINS_DIR}/../lib ${QT_PLUGINS_DIR} ${CMAKE_INSTALL_PREFIX}/lib)

if(NOT EXISTS "${TFW_DEVELOPMENT_DIR}/${APP_CONTENTS_DIR}/Resources/data" AND FALSE)
#
# TODO this is not compatible with install target because also wanna create symlinks
#
    ADD_CUSTOM_TARGET(${PROJECT_NAME}-create-symlinks ALL
                  COMMAND ${CMAKE_COMMAND} -E create_symlink
                  ${TFW_DEVELOPMENT_DIR}/data
                  ${TFW_DEVELOPMENT_DIR}/${APP_CONTENTS_DIR}/Resources/data)

endif()
    configure_file(${CMAKE_CURRENT_LIST_DIR}/qt.conf.in
                      ${TFW_DEVELOPMENT_DIR}/${APP_CONTENTS_DIR}/Resources/qt.conf @ONLY)

    install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX})

    if(OPT_COMMUNITY_BUILD)
        install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/resources/compare DESTINATION ${APP_CONTENTS_DIR}/Resources)
        install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/resources/image DESTINATION ${APP_CONTENTS_DIR}/Resources)
        install(DIRECTORY ${TFW_PACKAGE_DIR}/localization DESTINATION ${APP_CONTENTS_DIR}/Resources)
    endif()
if(NO) # NOTE this is a backup if the fixup doesn't works
        get_filename_component(print_dir ${QT_PLUGINS_DIR}/../lib/QtPrintSupport.framework REALPATH)
        install(DIRECTORY ${print_dir} DESTINATION ${APP_PLUGINS_DIR}/../Frameworks/ )

        get_filename_component(core_dir ${QT_PLUGINS_DIR}/../lib/QtCore.framework REALPATH)
        install(DIRECTORY ${core_dir} DESTINATION ${APP_PLUGINS_DIR}/../Frameworks/ )

        get_filename_component(gui_dir ${QT_PLUGINS_DIR}/../lib/QtGui.framework REALPATH)
        install(DIRECTORY ${gui_dir} DESTINATION ${APP_PLUGINS_DIR}/../Frameworks/ )


        get_filename_component(opengl_dir ${QT_PLUGINS_DIR}/../lib/QtOpenGL.framework REALPATH)
        install(DIRECTORY ${opengl_dir} DESTINATION ${APP_PLUGINS_DIR}/../Frameworks/ )

        get_filename_component(widgets_dir ${QT_PLUGINS_DIR}/../lib/QtWidgets.framework REALPATH)
        install(DIRECTORY ${widgets_dir} DESTINATION ${APP_PLUGINS_DIR}/../Frameworks/ )


        get_filename_component(xml_dir ${QT_PLUGINS_DIR}/../lib/QtXml.framework REALPATH)
        install(DIRECTORY ${xml_dir} DESTINATION ${APP_PLUGINS_DIR}/../Frameworks/ )
endif()

    install(FILES "${QT_PLUGINS_DIR}/platforms/libqcocoa.dylib" DESTINATION ${APP_PLUGINS_DIR}/platforms)
    install(FILES "${QT_PLUGINS_DIR}/imageformats/libqjpeg.dylib" DESTINATION ${APP_PLUGINS_DIR}/imageformats)
    if(EXISTS ${TFW_PACKAGE_DIR}/plugins) # when benchmarks are skipped
        install(DIRECTORY ${TFW_PACKAGE_DIR}/plugins DESTINATION ${APP_CONTENTS_DIR}/Resources)
    endif()
    install(CODE "
        cmake_policy(SET CMP0009 NEW)
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${APP_CONTENTS_DIR}/Resources/qt.conf\" \"[Paths]\nPlugins = PlugIns\n\")
        file(GLOB_RECURSE QT_PLUGIN_LIBS \"\${CMAKE_INSTALL_PREFIX}/${APP_CONTENTS_DIR}/PlugIns/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
        file(GLOB_RECURSE PLUGIN_LIBS \"\${CMAKE_INSTALL_PREFIX}/${APP_CONTENTS_DIR}/Resources/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")

        # This is needed to copy the whole bundle. Apple rejected apps without proper Framework structure
        # Use custom BundleUtilities which uses 'cp -a' because we need symlinks in the bundle.
        # Original BundleUtilities uses cmake -E copy_directory which follows symlinks by default
        # see cmake issues:
        #   http://www.cmake.org/Bug/view.php?id=14609
        #   http://www.cmake.org/Bug/view.php?id=13204
        set(BU_COPY_FULL_FRAMEWORK_CONTENTS ON)
        include(${CMAKE_CURRENT_LIST_DIR}/../frameworks/cmake-utils/cmake/BundleUtilities_KISHONTI.cmake)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/${APP_BUNDLE}\" \"\${QT_PLUGIN_LIBS};\${PLUGIN_LIBS}\" \"${DIRS}\")
        ")
    # install data files after fixup because fixup checks all files if it is executable (slow)
    # don't copy config directory (they are included in the app as resources)
    install(FILES ${RESOURCE_DIR}/images/icon.icns DESTINATION ${APP_CONTENTS_DIR}/Resources/)
    if(EXISTS ${TFW_PACKAGE_DIR}/images)
         install(DIRECTORY ${TFW_PACKAGE_DIR}/images DESTINATION ${APP_CONTENTS_DIR}/Resources/)
    endif()
    if(NOT OPT_COMMUNITY_BUILD OR BUNDLE_DATA)
      if(EXISTS ${TFW_PACKAGE_DIR}/config)
         install(DIRECTORY ${TFW_PACKAGE_DIR}/config DESTINATION ${APP_CONTENTS_DIR}/Resources/)
      endif()
      if(EXISTS ${TFW_PACKAGE_DIR}/data)
         install(DIRECTORY ${TFW_PACKAGE_DIR}/data DESTINATION ${APP_CONTENTS_DIR}/Resources/)
      endif()
    endif()

    if (STORE_VERSION)
        install(CODE "
            # sign the package, and extract the debug symbols for crash reporter
            file(MAKE_DIRECTORY ${TFW_PACKAGE_DIR}/dSYMs)
            file(GLOB_RECURSE dylibs \${CMAKE_INSTALL_PREFIX}/${APP_CONTENTS_DIR}/*.dylib)
            execute_process(COMMAND /usr/libexec/PListBuddy -c \"Set BuildMachineOSBuild 13A603\" \${CMAKE_INSTALL_PREFIX}/${APP_CONTENTS_DIR}/Info.plist)
            foreach(dylib \${dylibs})
                get_filename_component(name \${dylib} NAME_WE)
                execute_process(COMMAND codesign --options=runtime --deep --force --verify --timestamp --verbose --sign \"${DEVELOPER_CODE_SIGN_IDENTITY}\" \"\${dylib}\"
                                COMMAND dsymutil \"\${dylib}\" -o ${TFW_PACKAGE_DIR}/dSYMs/\${name}.dSYM)
            endforeach()
            file(GLOB frameworks \${CMAKE_INSTALL_PREFIX}/${APP_CONTENTS_DIR}/Frameworks/*.framework)
            foreach(framework \${frameworks})
                get_filename_component(name \"\${framework}\" NAME_WE)
                execute_process(COMMAND find \"\${framework}\" -name *_debug* -delete)
                execute_process(COMMAND find \"\${framework}\" -name *.prl -delete)
                execute_process(COMMAND rm -rf \"\${framework}/Headers\"
                                COMMAND rm -rf \"\${framework}/Versions/Current/Headers\")
                execute_process(COMMAND codesign --options=runtime --deep --force --verify --timestamp --verbose --sign \"${DEVELOPER_CODE_SIGN_IDENTITY}\" \"\${framework}/\${name}\")
            endforeach()
            set(app \${CMAKE_INSTALL_PREFIX}/${APP_CONTENTS_DIR}/MacOS/${PROJECT_NAME})
            execute_process(COMMAND codesign --options=runtime --deep --force --verify --timestamp --verbose --sign \"${DEVELOPER_CODE_SIGN_IDENTITY}\" \"\${app}\"
                            COMMAND dsymutil \"\${app}\" -o ${TFW_PACKAGE_DIR}/dSYMs/${PROJECT_NAME}.dSYM)

            execute_process(COMMAND rm -rf \"\${CMAKE_INSTALL_PREFIX}/${PRODUCT_NAME}.app\")
            execute_process(COMMAND mv \"\${CMAKE_INSTALL_PREFIX}/${APP_BUNDLE}\" \"\${CMAKE_INSTALL_PREFIX}/${PRODUCT_NAME}.app\")
            execute_process(COMMAND codesign --options=runtime --deep --force --verify --verbose --timestamp --sign \"${DEVELOPER_CODE_SIGN_IDENTITY}\" --entitlements \"${CMAKE_CURRENT_LIST_DIR}/resources/MacOSXApp.entitlements\" \"\${CMAKE_INSTALL_PREFIX}/${PRODUCT_NAME}.app\")

            # create signed .pkg for the store
            execute_process(COMMAND productbuild --timestamp --component \"\${CMAKE_INSTALL_PREFIX}/${PRODUCT_NAME}.app\" /Applications/ --sign \"${INSTALLER_CODE_SIGN_IDENTITY}\" \"${TFW_PACKAGE_DIR}/${CPACK_PACKAGE_FILE_NAME}.pkg\")
            if(NOT \"$ENV{ARCHIVE_ROOT}\" STREQUAL \"\")
                execute_process(COMMAND ${CMAKE_COMMAND} -E copy \"${TFW_PACKAGE_DIR}/${CPACK_PACKAGE_FILE_NAME}.pkg\" \"$ENV{ARCHIVE_ROOT}\")
	    endif()
            ")
    else()
        install(CODE "message(STATUS \"STORE_VERSION not set. No need for code signing\")")
    endif()
endif()

#Packaging

set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/resources/eula.txt)
set(CPACK_PACKAGE_VENDOR "Kishonti Ltd")
set(CPACK_PACKAGE_NAME "${PRODUCT_NAME}")
set(CPACK_PACKAGE_VERSION_MAJOR "${BENCHMARK_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${BENCHMARK_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${BENCHMARK_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${PRODUCT_VERSION}")
set(CPACK_PACKAGE_CONTACT "Kishonti Ltd <support@${CONTACT_DOMAIN}>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PRODUCT_NAME} Benchmark")
set(CPACK_OUTPUT_FILE_PREFIX "$ENV{ARCHIVE_ROOT}")
set(CPACK_STRIP_FILES TRUE)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_RPM_PACKAGE_AUTOREQPROV OFF)
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST
    /usr/share/applications
    /usr
    /usr/lib
    /usr/share
    /usr/share/doc)

if(WIN32)
    if ("${OPT_CONFIG}" STREQUAL Debug)
        get_target_property(POSTFIX ${PROJECT_NAME} DEBUG_POSTFIX)
    endif()
    set(EXECUTABLE_NAME ${PROJECT_NAME}${POSTFIX})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Kishonti Ltd\\\\${PRODUCT_NAME}")
    set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${PRODUCT_NAME}")
    set(CPACK_PACKAGE_EXECUTABLES "${EXECUTABLE_NAME}" "${PRODUCT_NAME}")
    if(CMAKE_CL_64)
        set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    else()
        set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
    endif()
    set(CPACK_NSIS_PACKAGE_NAME "${PRODUCT_NAME}")
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY "bin")
    set(CPACK_NSIS_DISPLAY_NAME "${PRODUCT_NAME}")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${EXECUTABLE_NAME}.exe")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS "Exec '$INSTDIR\\\\Uninstall.exe /S'")
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "CreateShortCut \\\"$DESKTOP\\\\${PRODUCT_NAME}.lnk\\\" \\\"$INSTDIR\\\\\\\\bin\\\\\\\\${EXECUTABLE_NAME}.exe\\\"")
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        RMDir /r '$LOCALAPPDATA\\\\Kishonti Ltd\\\\${PRODUCT_NAME}'
        Delete  '$DESKTOP\\\\${PRODUCT_NAME}.lnk'
    ")
    if(OPT_COMMUNITY_BUILD)
        set(CPACK_GENERATOR NSIS)
    else()
        set(CPACK_GENERATOR ZIP)
    endif()
elseif(UNIX AND NOT APPLE)
    set(DEPENDENCIES
       # "qt5core5a (>= ${QT_VERSION}),"
       # "qt5gui5 (>= ${QT_VERSION}),"
       # "qt5widgets5 (>= ${QT_VERSION}),"
       # "qt5opengl5 (>= ${QT_VERSION}),"
       # "qt5xml5 (>= ${QT_VERSION})"
    )

    string(REPLACE ";" " " CPACK_DEBIAN_PACKAGE_DEPENDS "${DEPENDENCIES}")
    set(CPACK_DEBIAN_PACKAGE_NAME "${PACKAGE_NAME}")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${PRODUCT_NAME}
 GFXBench is a comprehensive cross-platform and cross-API 3D graphics benchmark.")

    set(CPACK_EXECUTABLE_NAME "${PRODUCT_ID}")
    string(REPLACE ";" " " CPACK_RPM_PACKAGE_REQUIRES "${DEPENDENCIES}")
    string(REPLACE "(" "" CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}")
    string(REPLACE ")" "" CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}")

    if(OPT_COMMUNITY_BUILD)
    if( CMAKE_SIZEOF_P MATCHES 8)
        set(LIB1 libicui18n.so.56)
        set(LIB2 libicuuc.so.56)
        set(LIB3 libicudata.so.56)
    else()
        set(LIB1 libicui18n.so.54)
        set(LIB2 libicuuc.so.54)
        set(LIB3 libicudata.so.54)
    endif()

        get_filename_component(start_scripts ${CMAKE_CURRENT_LIST_DIR}/start.sh REALPATH)
        get_filename_component(core_dir ${QT_LIBRARY_DIR}/libQt5Core${DEBUG_POSTFIX}.so REALPATH)
        get_filename_component(gui_dir ${QT_LIBRARY_DIR}/libQt5Gui${DEBUG_POSTFIX}.so REALPATH)
        get_filename_component(opengl_dir ${QT_LIBRARY_DIR}/libQt5OpenGL${DEBUG_POSTFIX}.so REALPATH)
        get_filename_component(widgets_dir ${QT_LIBRARY_DIR}/libQt5Widgets${DEBUG_POSTFIX}.so REALPATH)
        get_filename_component(icui_dir ${QT_LIBRARY_DIR}/${LIB1} REALPATH)
        get_filename_component(icuu_dir ${QT_LIBRARY_DIR}/${LIB2} REALPATH)
        get_filename_component(icudata_dir ${QT_LIBRARY_DIR}/${LIB3} REALPATH)
        get_filename_component(xml_dir ${QT_LIBRARY_DIR}/libQt5Xml.so.5 REALPATH)
        get_filename_component(dbus_dir ${QT_LIBRARY_DIR}/libQt5DBus.so.5 REALPATH)
        get_filename_component(xcbqpa_dir ${QT_LIBRARY_DIR}/libQt5XcbQpa.so.5 REALPATH)
        get_filename_component(xcb_dir ${QT_PLUGINS_DIR}/platforms/libqxcb${DEBUG_POSTFIX}.so REALPATH)
        install(FILES ${start_scripts} DESTINATION . RENAME start.sh)
        install(FILES ${core_dir} DESTINATION . RENAME libQt5Core${DEBUG_POSTFIX}.so.5 )
        install(FILES ${gui_dir} DESTINATION . RENAME libQt5Gui${DEBUG_POSTFIX}.so.5 )
        install(FILES ${opengl_dir} DESTINATION . RENAME libQt5OpenGL${DEBUG_POSTFIX}.so.5 )
        install(FILES ${widgets_dir} DESTINATION . RENAME libQt5Widgets${DEBUG_POSTFIX}.so.5 )
        install(FILES ${xml_dir} DESTINATION . RENAME libQt5Xml${DEBUG_POSTFIX}.so.5 )
        install(FILES ${icui_dir} DESTINATION . RENAME ${LIB1} )
        install(FILES ${icuu_dir} DESTINATION . RENAME ${LIB2} )
        install(FILES ${icudata_dir} DESTINATION . RENAME ${LIB3} )
        install(FILES ${dbus_dir} DESTINATION . RENAME libQt5DBus.so.5 )
        install(FILES ${dbus_dir} DESTINATION . RENAME platforms/libQt5DBus.so.5 )
        install(FILES ${xcbqpa_dir} DESTINATION . RENAME platforms/libQt5XcbQpa.so.5 )
        install(FILES ${xcb_dir} DESTINATION . RENAME platforms/libqxcb${DEBUG_POSTFIX}.so )
        install(FILES resources/${PRODUCT_ID}/${PRODUCT_ID}.desktop DESTINATION . RENAME ${PRODUCT_ID}.desktop )
        install(FILES resources/${PRODUCT_ID}/images/icon.png DESTINATION . RENAME ${PRODUCT_ID}.png )
        set(CPACK_GENERATOR STGZ)
    else()
        set(CPACK_GENERATOR TGZ)
    endif()
else()
    set(CPACK_GENERATOR TGZ)
endif()

if(NOT (APPLE AND OPT_COMMUNITY_BUILD))
    include(CPack)
endif()
