set(VISUALUI_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(UICLASSES ${VISUALUI_ROOT}/src)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${UICLASSES}
    ${UICLASSES}/core
    ${UICLASSES}/common
    ${UICLASSES}/graphics
    ${UICLASSES}/layout
    ${UICLASSES}/widgets
    ${UICLASSES}/dialogs
)

set(VISUALUI_HEADERS
    ${UICLASSES}/common/uidefines.h
    ${UICLASSES}/common/uiplatformtypes.h
    ${UICLASSES}/common/uiutils.h
    ${UICLASSES}/common/uicommon.h
    ${UICLASSES}/graphics/uimetrics.h
    ${UICLASSES}/graphics/uipalette.h
    ${UICLASSES}/graphics/uidrawningengine.h
    ${UICLASSES}/graphics/uidrawingsurface.h
    ${UICLASSES}/graphics/uistyle.h
    ${UICLASSES}/graphics/uigeometryanimation.h
    ${UICLASSES}/graphics/uiopacityanimation.h
    ${UICLASSES}/graphics/uiscalaranimation.h
    ${UICLASSES}/graphics/uiconhandler.h
    ${UICLASSES}/graphics/uidraghandler.h
    ${UICLASSES}/graphics/uitooltiphandler.h
    ${UICLASSES}/graphics/uipixmap.h
    ${UICLASSES}/core/uisignal.h
    ${UICLASSES}/core/uiobject.h
    ${UICLASSES}/core/uicursor.h
    ${UICLASSES}/core/uiapplication.h
    ${UICLASSES}/core/uithread.h
    ${UICLASSES}/core/uifontmetrics.h
    ${UICLASSES}/core/uitimer.h
    ${UICLASSES}/core/uieventloop.h
    ${UICLASSES}/core/uixmldocument.h
    ${UICLASSES}/dialogs/uipopupmessage.h
    ${UICLASSES}/dialogs/uidialog.h
    ${UICLASSES}/dialogs/uifiledialog.h
    ${UICLASSES}/widgets/uiwindow.h
    ${UICLASSES}/widgets/uiabstractwindow.h
    ${UICLASSES}/widgets/uiabstractpopup.h
    ${UICLASSES}/widgets/uiabstractscrollarea.h
    ${UICLASSES}/widgets/uitooltip.h
    ${UICLASSES}/widgets/uimenu.h
    ${UICLASSES}/widgets/uiwidget.h
    ${UICLASSES}/widgets/uilabel.h
    ${UICLASSES}/widgets/uicaption.h
    ${UICLASSES}/widgets/uiabstractbutton.h
    ${UICLASSES}/widgets/uibutton.h
    ${UICLASSES}/widgets/uitoolbutton.h
    ${UICLASSES}/widgets/uicheckbox.h
    ${UICLASSES}/widgets/uiradiobutton.h
    ${UICLASSES}/widgets/uitogglebutton.h
    ${UICLASSES}/widgets/uiprogressbar.h
    ${UICLASSES}/widgets/uiscrollbar.h
    ${UICLASSES}/widgets/uiscrollarea.h
    ${UICLASSES}/widgets/uilistview.h
    ${UICLASSES}/widgets/uilineedit.h
    # ${UICLASSES}/widgets/uitextedit.h
    ${UICLASSES}/widgets/uicombobox.h
    ${UICLASSES}/layout/uispacer.h
    ${UICLASSES}/layout/uilayoutitem.h
    ${UICLASSES}/layout/uilayout.h
    ${UICLASSES}/layout/uiboxlayout.h
)

set(VISUALUI_SOURCES
    ${UICLASSES}/common/uiutils.cpp
    ${UICLASSES}/common/uicommon.cpp
    ${UICLASSES}/graphics/uimetrics.cpp
    ${UICLASSES}/graphics/uipalette.cpp
    ${UICLASSES}/graphics/uidrawningengine.cpp
    ${UICLASSES}/graphics/uidrawingsurface.cpp
    ${UICLASSES}/graphics/uistyle.cpp
    ${UICLASSES}/graphics/uigeometryanimation.cpp
    ${UICLASSES}/graphics/uiopacityanimation.cpp
    ${UICLASSES}/graphics/uiscalaranimation.cpp
    ${UICLASSES}/graphics/uiconhandler.cpp
    ${UICLASSES}/graphics/uidraghandler.cpp
    ${UICLASSES}/graphics/uitooltiphandler.cpp
    ${UICLASSES}/graphics/uipixmap.cpp
    ${UICLASSES}/core/uiobject.cpp
    ${UICLASSES}/core/uicursor.cpp
    ${UICLASSES}/core/uiapplication.cpp
    ${UICLASSES}/core/uifontmetrics.cpp
    ${UICLASSES}/core/uitimer.cpp
    ${UICLASSES}/core/uieventloop.cpp
    ${UICLASSES}/core/uixmldocument.cpp
    ${UICLASSES}/dialogs/uipopupmessage.cpp
    ${UICLASSES}/dialogs/uidialog.cpp
    ${UICLASSES}/dialogs/uifiledialog.cpp
    ${UICLASSES}/widgets/uiwindow.cpp
    ${UICLASSES}/widgets/uiabstractwindow.cpp
    ${UICLASSES}/widgets/uiabstractpopup.cpp
    ${UICLASSES}/widgets/uiabstractscrollarea.cpp
    ${UICLASSES}/widgets/uitooltip.cpp
    ${UICLASSES}/widgets/uimenu.cpp
    ${UICLASSES}/widgets/uiwidget.cpp
    ${UICLASSES}/widgets/uilabel.cpp
    ${UICLASSES}/widgets/uicaption.cpp
    ${UICLASSES}/widgets/uiabstractbutton.cpp
    ${UICLASSES}/widgets/uibutton.cpp
    ${UICLASSES}/widgets/uitoolbutton.cpp
    ${UICLASSES}/widgets/uicheckbox.cpp
    ${UICLASSES}/widgets/uiradiobutton.cpp
    ${UICLASSES}/widgets/uitogglebutton.cpp
    ${UICLASSES}/widgets/uiprogressbar.cpp
    ${UICLASSES}/widgets/uiscrollbar.cpp
    ${UICLASSES}/widgets/uiscrollarea.cpp
    ${UICLASSES}/widgets/uilistview.cpp
    ${UICLASSES}/widgets/uilineedit.cpp
    # ${UICLASSES}/widgets/uitextedit.cpp
    ${UICLASSES}/widgets/uicombobox.cpp
    ${UICLASSES}/layout/uispacer.cpp
    ${UICLASSES}/layout/uilayoutitem.cpp
    ${UICLASSES}/layout/uilayout.cpp
    ${UICLASSES}/layout/uiboxlayout.cpp
)

if (UNIX AND NOT APPLE)
    set(VISUALUI_HEADERS
        ${VISUALUI_HEADERS}
        ${UICLASSES}/widgets/linux/gtkcaret.h
    )

    set(VISUALUI_SOURCES
        ${VISUALUI_SOURCES}
        ${UICLASSES}/widgets/linux/gtkcaret.c
    )
endif()

target_sources(${PROJECT_NAME} PRIVATE ${VISUALUI_HEADERS} ${VISUALUI_SOURCES})

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE UNICODE)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS,5.02")
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS,5.01")
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE gdi32 gdiplus user32 shell32 shlwapi advapi32 ole32 comctl32 winmm)

elseif (UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    find_package(Threads REQUIRED)
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
        X11
        ${GTK3_LIBRARIES}
        ${LIBXML2_LIBRARIES}
        ${GIO_LIBRARIES}
        ${RSVG_LIBRARIES}
        Threads::Threads
    )
endif()
