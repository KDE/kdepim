include_directories( ${GPGME_INCLUDES} ${Boost_INCLUDE_DIR} )
KDE4_NO_ENABLE_FINAL(libkleo)
if (KDEPIM_ONLY_KLEO)
  add_definitions( -DKDEPIM_ONLY_KLEO )
endif (KDEPIM_ONLY_KLEO)

remove_definitions( -DQT_NO_STL )
add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS -DQT_STL )
add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )
#parts of Boost don't build with the gcc pedantic option, so remove that option
string(REPLACE "-pedantic" "" FOO ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "${FOO}")

add_subdirectory( pics )
add_subdirectory( backends )
add_subdirectory( tests )

########### next target ###############
set(libkleo_backend_qgpgme_SRCS
    backends/qgpgme/gnupgprocessbase.cpp
    backends/qgpgme/qgpgmeprogresstokenmapper.cpp
    backends/qgpgme/qgpgmebackend.cpp
    backends/qgpgme/threadedjobmixin.cpp
    backends/qgpgme/qgpgmekeylistjob.cpp
    backends/qgpgme/qgpgmekeygenerationjob.cpp
    backends/qgpgme/qgpgmeimportjob.cpp
    backends/qgpgme/qgpgmeimportfromkeyserverjob.cpp
    backends/qgpgme/qgpgmeexportjob.cpp
    backends/qgpgme/qgpgmesecretkeyexportjob.cpp
    backends/qgpgme/qgpgmechangeexpiryjob.cpp
    backends/qgpgme/qgpgmechangeownertrustjob.cpp
    backends/qgpgme/qgpgmedownloadjob.cpp
    backends/qgpgme/qgpgmedeletejob.cpp
    backends/qgpgme/qgpgmeencryptjob.cpp
    backends/qgpgme/qgpgmedecryptjob.cpp
    backends/qgpgme/qgpgmesignjob.cpp
    backends/qgpgme/qgpgmesignkeyjob.cpp
    backends/qgpgme/qgpgmeadduseridjob.cpp
    backends/qgpgme/qgpgmeverifydetachedjob.cpp
    backends/qgpgme/qgpgmeverifyopaquejob.cpp
    backends/qgpgme/qgpgmesignencryptjob.cpp
    backends/qgpgme/qgpgmedecryptverifyjob.cpp
    backends/qgpgme/qgpgmecryptoconfig.cpp
    backends/qgpgme/qgpgmerefreshkeysjob.cpp
    )


set(libkleo_backend_chiasmus_SRCS
    backends/chiasmus/config_data.c
    backends/chiasmus/obtainkeysjob.cpp
    backends/chiasmus/symcryptrunprocessbase.cpp
    backends/chiasmus/chiasmusjob.cpp
    backends/chiasmus/chiasmuslibrary.cpp
    backends/chiasmus/chiasmusbackend.cpp
    )


set(libkleo_core_SRCS
   kleo/cryptobackend.cpp
   kleo/cryptobackendfactory.cpp
   kleo/enum.cpp
   kleo/dn.cpp
   kleo/job.cpp
   kleo/multideletejob.cpp
   kleo/hierarchicalkeylistjob.cpp
   kleo/kconfigbasedkeyfilter.cpp
   kleo/keyfiltermanager.cpp
   )

set(libkleo_ui_SRCS    # make this a separate lib.
   ui/keylistview.cpp
   ui/keyselectiondialog.cpp
   ui/keyrequester.cpp
   ui/keyapprovaldialog.cpp
   ui/backendconfigwidget.cpp
)
set(libkleo_ui_common_SRCS
   ui/kdhorizontalline.cpp
   ui/filenamerequester.cpp
   ui/messagebox.cpp
   ui/progressbar.cpp
   ui/progressdialog.cpp
   ui/cryptoconfigmodule.cpp
   ui/cryptoconfigdialog.cpp
   ui/directoryserviceswidget.cpp
   ui/adddirectoryservicedialogimpl.cpp
   ui/dnattributeorderconfigwidget.cpp
)

kde4_add_ui_files(libkleo_ui_common_SRCS
    ui/adddirectoryservicedialog.ui
    ui/directoryserviceswidget.ui
)

set(kleo_LIB_SRCS ${libkleo_core_SRCS} ${libkleo_ui_common_SRCS} ${libkleo_backend_qgpgme_SRCS})
set(kleo_LIB_LIBS ${KDE4_KDEUI_LIBS} ${QGPGME_LIBRARIES} ${QT_QT3SUPPORT_LIBRARY} )
if ( NOT KDEPIM_ONLY_KLEO )
   set( kleo_LIB_SRCS ${kleo_LIB_SRCS} ${libkleo_ui_SRCS} ${libkleo_backend_chiasmus_SRCS} )
   set( kleo_LIB_LIBS ${kleo_LIB_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} )
endif ( NOT KDEPIM_ONLY_KLEO)

kde4_add_library(kleo SHARED ${kleo_LIB_SRCS})
if(WIN32)
target_link_libraries(kleo ${kleo_LIB_LIBS}  ${GPGME_VANILLA_LIBRARIES} )
else(WIN32)
target_link_libraries(kleo ${kleo_LIB_LIBS} )
endif(WIN32)
set_target_properties(kleo PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kleo ${INSTALL_TARGETS_DEFAULT_ARGS})

install ( FILES libkleopatrarc.desktop DESTINATION ${CONFIG_INSTALL_DIR} RENAME libkleopatrarc )
