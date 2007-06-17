include_directories( ${GPGME_INCLUDES} )


########### next target ###############
set(dataprovidertest_SRCS dataprovidertest.cpp )

kde4_automoc(${dataprovidertest_SRCS})

kde4_add_test_executable(dataprovidertest ${dataprovidertest_SRCS})

target_link_libraries(dataprovidertest  qgpgme ${KDE4_KDECORE_LIBS} )


