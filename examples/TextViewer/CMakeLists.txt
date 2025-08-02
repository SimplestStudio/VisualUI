cmake_minimum_required(VERSION 3.5)
project(TextViewer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "" FORCE)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_definitions(-DVISUALUI_STATIC)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.cpp
)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.h
)

set(OTHER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/res/styles/styles.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/res/styles/themes.xml
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

include(${CMAKE_CURRENT_SOURCE_DIR}/../../VisualUI.cmake)

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    set(OTHER_FILES
        ${OTHER_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/res/manifest/TextViewer.exe.manifest
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

