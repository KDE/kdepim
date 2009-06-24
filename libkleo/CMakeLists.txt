include_directories( ${CMAKE_BINARY_DIR}/kleo/backends/chiasmus/ ${GPGME_INCLUDES} ${Boost_INCLUDE_DIR}  )

if (KDEPIM_ONLY_KLEO)
  add_definitions( -DKDEPIM_ONLY_KLEO )
endif (KDEPIM_ONLY_KLEO)

remove_definitions( -DQT_NO_STL )
add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS -DQT_STL )
add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )

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

set(libkleo_compat_SRCS # make this a separate lib.
   kleo/cryptplug.cpp
   kleo/cryptplugwrapper.cpp
   kleo/cryptplugwrapperlist.cpp
   kleo/cryptplugfactory.cpp )

kde4_add_ui_files(libkleo_ui_common_SRCS
	ui/adddirectoryservicedialog.ui
	ui/directoryserviceswidget.ui
)

set(kleo_LIB_SRCS ${libkleo_core_SRCS} ${libkleo_ui_common_SRCS} ${libkleo_backend_qgpgme_SRCS})
set(kleo_LIB_LIBS ${KDE4_KDEUI_LIBS} ${QGPGME_LIBRARIES} ${QT_QT3SUPPORT_LIBRARY} )
if ( NOT KDEPIM_ONLY_KLEO )
   set( kleo_LIB_SRCS ${kleo_LIB_SRCS} ${libkleo_ui_SRCS} ${libkleo_backend_chiasmus_SRCS} )
   set( kleo_LIB_LIBS ${kleo_LIB_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} )
   if ( NOT KLEO_NO_COMPAT )
     set( kleo_LIB_SRCS ${kleo_LIB_SRCS} ${libkleo_compat_SRCS} )
   endif ( NOT KLEO_NO_COMPAT )
endif ( NOT KDEPIM_ONLY_KLEO)

kde4_add_library(kleo SHARED ${kleo_LIB_SRCS})
if(WIN32)
target_link_libraries(kleo ${kleo_LIB_LIBS}  ${GPGME_VANILLA_LIBRARIES} )
else(WIN32)
target_link_libraries(kleo ${kleo_LIB_LIBS} )
endif(WIN32)
set_target_properties(kleo PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kleo ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
	kleo/cryptplug.h
	kleo/cryptplugfactory.h
	kleo/cryptplugwrapper.h
	kleo/cryptplugwrapperlist.h
	kleo/kleo_export.h
	kleo/enum.h
	kleo/oidmap.h
	kleo/cryptobackend.h
	kleo/cryptobackendfactory.h
	kleo/cryptoconfig.h
	kleo/dn.h
	kleo/job.h
	kleo/keylistjob.h
	kleo/keygenerationjob.h
        kleo/abstractimportjob.h
	kleo/importjob.h
	kleo/importfromkeyserverjob.h
	kleo/exportjob.h
	kleo/changeexpiryjob.h
	kleo/changeownertrustjob.h
	kleo/downloadjob.h
	kleo/deletejob.h
	kleo/encryptjob.h
	kleo/decryptjob.h
	kleo/signjob.h
	kleo/specialjob.h
	kleo/verifydetachedjob.h
	kleo/verifyopaquejob.h
	kleo/decryptverifyjob.h
	kleo/signencryptjob.h
	kleo/signkeyjob.h
        kleo/adduseridjob.h
	kleo/refreshkeysjob.h
	kleo/multideletejob.h
	kleo/hierarchicalkeylistjob.h
	kleo/keyfilter.h
	kleo/keyfiltermanager.h
	DESTINATION ${INCLUDE_INSTALL_DIR}/kleo COMPONENT Devel)

install ( FILES
	ui/kdhorizontalline.h
	ui/messagebox.h
	ui/progressbar.h
	ui/progressdialog.h
	ui/keylistview.h
	ui/keyselectiondialog.h
	ui/keyrequester.h
	ui/keyapprovaldialog.h
	ui/backendconfigwidget.h
	ui/dnattributeorderconfigwidget.h
	ui/cryptoconfigmodule.h
	ui/cryptoconfigdialog.h
	ui/directoryserviceswidget.h
	DESTINATION ${INCLUDE_INSTALL_DIR}/kleo/ui COMPONENT Devel)

install ( FILES libkleopatrarc.desktop DESTINATION ${CONFIG_INSTALL_DIR} RENAME libkleopatrarc )
