include_directories(${CMAKE_SOURCE_DIR}/libkpgp/ ${CMAKE_BINARY_DIR}/libkleo/backends/chiasmus/  ${GPGME_INCLUDES} ${CMAKE_SOURCE_DIR}/gpgmepp/ )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

add_subdirectory( pics )
add_subdirectory( backends )
add_subdirectory( tests )


########### next target ###############
set(libkleopatra_backend_qgpgme_SRCS
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/gnupgprocessbase.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmeprogresstokenmapper.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmebackend.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmejob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmekeylistjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmekeygenerationjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmeimportjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmeexportjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmesecretkeyexportjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmedownloadjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmedeletejob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmeencryptjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmedecryptjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmesignjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmeverifydetachedjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmeverifyopaquejob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmesignencryptjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmedecryptverifyjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmecryptoconfig.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmerefreshkeysjob.cpp
	)


set(libkleopatra_backend_chiasmus_SRCS 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/config_data.c 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/obtainkeysjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/symcryptrunprocessbase.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/chiasmusjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/chiasmuslibrary.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/chiasmusbackend.cpp
	)


set(libkleopatra_core_SRCS
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/cryptobackend.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/cryptobackendfactory.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/enum.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/dn.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/job.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/multideletejob.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/hierarchicalkeylistjob.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/kconfigbasedkeyfilter.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/keyfiltermanager.cpp
   )

set(libkleopatra_ui_SRCS
   ${CMAKE_SOURCE_DIR}/libkleo/ui/kdhorizontalline.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/progressbar.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/progressdialog.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/keylistview.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/keyselectiondialog.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/keyrequester.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/keyapprovaldialog.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/backendconfigwidget.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/dnattributeorderconfigwidget.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/cryptoconfigmodule.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/cryptoconfigdialog.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/directoryserviceswidget.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/adddirectoryservicedialogimpl.cpp
)

kde4_add_ui_files(libkleopatra_ui_SRCS
	${CMAKE_SOURCE_DIR}/libkleo/ui/adddirectoryservicedialog.ui
	${CMAKE_SOURCE_DIR}/libkleo/ui/directoryserviceswidgetbase.ui
)

set(kleopatra_LIB_SRCS ${libkleopatra_core_SRCS} ${libkleopatra_ui_SRCS} ${libkleopatra_backend_chiasmus_SRCS} ${libkleopatra_backend_qgpgme_SRCS}
   cryptplug.cpp 
   cryptplugwrapper.cpp 
   cryptplugwrapperlist.cpp 
   cryptplugfactory.cpp )

kde4_automoc(${kleopatra_LIB_SRCS})

kde4_add_library(kleopatra SHARED ${kleopatra_LIB_SRCS})

target_link_libraries(kleopatra  ${KDE4_KDE3SUPPORT_LIBS} qgpgme  )

set_target_properties(kleopatra PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kleopatra  DESTINATION ${LIB_INSTALL_DIR})

install( FILES 
	kleo/enum.h  
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
	kleo/verifydetachedjob.h 
	kleo/verifyopaquejob.h  
	kleo/decryptverifyjob.h 
	kleo/signencryptjob.h 
	kleo/refreshkeysjob.h 
	kleo/multideletejob.h  
	kleo/hierarchicalkeylistjob.h  
	kleo/keyfilter.h 
	kleo/keyfiltermanager.h
	libkleo_export.h
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
	libkleo_export.h
	DESTINATION ${INCLUDE_INSTALL_DIR}/libkleoui)

