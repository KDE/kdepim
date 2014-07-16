set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DMAIL_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\"")

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/libkleo
  ${CMAKE_SOURCE_DIR}/templateparser/
  ${CMAKE_BINARY_DIR}/templateparser/
)
include ( ${CMAKE_SOURCE_DIR}/cmake/modules/kde4_handle_crypto_rpath_for_executable.cmake )

# convenience macro to add qtest unit tests
macro(add_templateparser_unittest _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME templateparser-${_name} ${_test})
  target_link_libraries(${_name}
    kleo
    templateparser
    messageviewer
    ${QGPGME_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY}
    ${KDE4_KIO_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  )
endmacro()

add_templateparser_unittest(templateparsertest.cpp)
kde4_handle_crypto_rpath_for_executable( templateparsertest )


set(templateeditor_gui_SRCS templateeditor_gui.cpp)
kde4_add_executable(templateeditor_gui TEST ${templateeditor_gui_SRCS})
target_link_libraries(templateeditor_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${QT_QTWEBKIT_LIBRARY}
  ${KDE4_KIO_LIBS}
  templateparser
)

set(templateconfigurewidget_gui_SRCS templateconfigurewidget_gui.cpp)
kde4_add_executable(templateconfigurewidget_gui TEST ${templateconfigurewidget_gui_SRCS})
target_link_libraries(templateconfigurewidget_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${QT_QTWEBKIT_LIBRARY}
  ${KDE4_KIO_LIBS}
  templateparser
)
