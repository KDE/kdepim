#add_subdirectory( tests )

include_directories(
        ${Boost_INCLUDE_DIR}
        ${AKONADI_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )


set(akonadi_serializer_sms_PART_SRCS akonadi_serializer_sms.cpp )
kde4_add_plugin(akonadi_serializer_sms ${akonadi_serializer_sms_PART_SRCS})
target_link_libraries(akonadi_serializer_sms ${KDE4_KDECORE_LIBS} kmobiletoolslib ${KDE4_AKONADI_LIBS})
install(TARGETS akonadi_serializer_sms DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES akonadi_serializer_sms.desktop DESTINATION ${DATA_INSTALL_DIR}/akonadi/plugins/serializer)

