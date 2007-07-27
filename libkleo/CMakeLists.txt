include_directories( ${CMAKE_BINARY_DIR}/kleo/backends/chiasmus/ ${GPGME_INCLUDES} )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

add_subdirectory( pics )
add_subdirectory( backends )
add_subdirectory( tests )


########### next target ###############
set(libkleo_backend_qgpgme_SRCS
	backends/qgpgme/gnupgprocessbase.cpp 
	backends/qgpgme/qgpgmeprogresstokenmapper.cpp 
	backends/qgpgme/qgpgmebackend.cpp 
	backends/qgpgme/qgpgmejob.cpp 
	backends/qgpgme/qgpgmekeylistjob.cpp 
	backends/qgpgme/qgpgmekeygenerationjob.cpp 
	backends/qgpgme/qgpgmeimportjob.cpp 
	backends/qgpgme/qgpgmeexportjob.cpp 
	backends/qgpgme/qgpgmesecretkeyexportjob.cpp 
	backends/qgpgme/qgpgmedownloadjob.cpp 
	backends/qgpgme/qgpgmedeletejob.cpp 
	backends/qgpgme/qgpgmeencryptjob.cpp 
	backends/qgpgme/qgpgmedecryptjob.cpp 
	backends/qgpgme/qgpgmesignjob.cpp 
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

set(libkleo_ui_SRCS
   ui/kdhorizontalline.cpp 
   ui/progressbar.cpp 
   ui/progressdialog.cpp 
   ui/keylistview.cpp 
   ui/keyselectiondialog.cpp 
   ui/keyrequester.cpp 
   ui/keyapprovaldialog.cpp 
   ui/backendconfigwidget.cpp 
   ui/dnattributeorderconfigwidget.cpp 
   ui/cryptoconfigmodule.cpp 
   ui/cryptoconfigdialog.cpp 
   ui/directoryserviceswidget.cpp 
   ui/adddirectoryservicedialogimpl.cpp
)

kde4_add_ui_files(libkleo_ui_SRCS
	ui/adddirectoryservicedialog.ui
	ui/directoryserviceswidgetbase.ui
)

set(kleo_LIB_SRCS ${libkleo_core_SRCS} ${libkleo_ui_SRCS} ${libkleo_backend_chiasmus_SRCS}
   kleo/cryptplug.cpp 
   kleo/cryptplugwrapper.cpp 
   kleo/cryptplugwrapperlist.cpp 
   kleo/cryptplugfactory.cpp )

set(kleo_LIB_SRCS ${kleo_LIB_SRCS} ${libkleo_backend_qgpgme_SRCS})

kde4_add_library(kleo SHARED ${kleo_LIB_SRCS})

target_link_libraries(kleo ${KDE4_KDE3SUPPORT_LIBS} qgpgme ${GPGME_LIBRARIES} ${QT_QT3SUPPORT_LIBRARY})

set_target_properties(kleo PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kleo DESTINATION ${LIB_INSTALL_DIR})

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
	kleo/importjob.h 
	kleo/exportjob.h 
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
	kleo/refreshkeysjob.h 
	kleo/multideletejob.h  
	kleo/hierarchicalkeylistjob.h  
	kleo/keyfilter.h 
	kleo/keyfiltermanager.h
	DESTINATION ${INCLUDE_INSTALL_DIR}/kleo)
	
install ( FILES
	ui/kdhorizontalline.h 
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
	DESTINATION ${INCLUDE_INSTALL_DIR}/kleo/ui)

