include_directories(
    ${CMAKE_SOURCE_DIR}/noteshared/
)

kde4_add_unit_test(noteeditorutilstest noteeditorutilstest.cpp )
target_link_libraries(noteeditorutilstest ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} noteshared)
