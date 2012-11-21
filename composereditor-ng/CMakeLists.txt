set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory(tests)

set(libcomposereditor_ng_SRCS 
    composereditor.cpp
)

kde4_add_library(composereditorng ${LIBRARY_TYPE} ${libcomposereditor_ng_SRCS})

target_link_libraries(composereditorng 
  ${KDE4_KDEWEBKIT_LIBRARY} ${QT_QTWEBKIT_LIBRARY} ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
)


set_target_properties( composereditorng PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS composereditorng ${INSTALL_TARGETS_DEFAULT_ARGS} )

#install( FILES
#         composereditor_export.h
#         composereditor.h
#         DESTINATION ${INCLUDE_INSTALL_DIR}/composereditorng COMPONENT Devel )

