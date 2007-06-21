#add_subdirectory( tests )

# include_directories(
#         ${CMAKE_SOURCE_DIR}/akonadi
#         ${Boost_INCLUDE_DIR}
# )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )


set(akonadi_serializer_sms_PART_SRCS akonadi_serializer_sms.cpp )
kde4_automoc(${akonadi_serializer_sms_PART_SRCS})
kde4_add_plugin(akonadi_serializer_sms WITH_PREFIX ${akonadi_serializer_sms_PART_SRCS})
target_link_libraries(akonadi_serializer_sms ${KDE4_KDECORE_LIBS} kmobiletools akonadi )
install(TARGETS akonadi_serializer_sms DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES akonadi_serializer_sms.desktop DESTINATION ${DATA_INSTALL_DIR}/akonadi/plugins/serializer)

