cmake_minimum_required(VERSION 3.5)
project(ImageViewer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "" FORCE)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_definitions(-DVISUALUI_STATIC)

set(VISUALUI_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(UICLASSES ${VISUALUI_ROOT}/src)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.cpp
)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui_mainwindow.h
)

set(OTHER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/res/styles/styles.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/res/styles/themes.xml
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${UICLASSES}
    ${UICLASSES}/core
    ${UICLASSES}/common
    ${UICLASSES}/graphics
    ${UICLASSES}/layout
    ${UICLASSES}/widgets
    ${UICLASSES}/dialogs
)

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE UNICODE)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS,5.02")
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS,5.01")
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "${VISUALUI_ROOT}/lib/build/$<CONFIG>/visualui.lib"
        gdiplus shlwapi comctl32 winmm
    )

elseif (UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
    pkg_check_modules(GIO REQUIRED gio-2.0)
    pkg_check_modules(RSVG REQUIRED librsvg-2.0)

    target_compile_definitions(${PROJECT_NAME} PRIVATE GTK3_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${GTK3_INCLUDE_DIRS}
        ${LIBXML2_INCLUDE_DIRS}
        ${GIO_INCLUDE_DIRS}
        ${RSVG_INCLUDE_DIRS}
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "${VISUALUI_ROOT}/lib/build/$<CONFIG>/libvisualui.a"
        X11
        ${GTK3_LIBRARIES}
        ${LIBXML2_LIBRARIES}
        ${GIO_LIBRARIES}
        ${RSVG_LIBRARIES}
    )
endif()

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    set(OTHER_FILES
        ${OTHER_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/res/manifest/ImageViewer.exe.manifest
    )
    set_source_files_properties(
        ${OTHER_FILES}
        PROPERTIES HEADER_FILE_ONLY TRUE
    )
    set(RC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/res/resource.rc)
    set(RESOURCE_FILES
        ${RC_FILE}
        ${OTHER_FILES}
    )
    target_sources(${PROJECT_NAME} PRIVATE ${RESOURCE_FILES})

elseif (UNIX AND NOT APPLE)
    set_source_files_properties(
        ${OTHER_FILES}
        PROPERTIES HEADER_FILE_ONLY TRUE
    )
    set(GLIB_RESOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/res/gresource.xml
    )
    set(RESOURCE_FILES
        ${GLIB_RESOURCE_FILES}
        ${OTHER_FILES}
    )
    set_source_files_properties(
        ${RESOURCE_FILES}
        PROPERTIES HEADER_FILE_ONLY TRUE
    )
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/res/gresource.c
        COMMAND glib-compile-resources --target=${CMAKE_CURRENT_SOURCE_DIR}/res/gresource.c --sourcedir=${CMAKE_CURRENT_SOURCE_DIR}/res --generate-source ${GLIB_RESOURCE_FILES}
        DEPENDS ${GLIB_RESOURCE_FILES}
        COMMENT "Generating GResource source file"
    )
    set(SOURCES
        ${SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/res/gresource.c
    )
    target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} ${RESOURCE_FILES})
endif()

source_group("Resource Files" FILES ${RESOURCE_FILES})
source_group("External Header Files" FILES ${VISUALUI_HEADERS})
source_group("External Source Files" FILES ${VISUALUI_SOURCES})

