include_directories( ${CMAKE_BINARY_DIR}/kleo/backends/chiasmus/ ${GPGME_INCLUDES} )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

add_subdirectory( pics )
add_subdirectory( backends )
add_subdirectory( tests )


########### next target ###############
set(libkleo_backend_qgpgme_SRCS
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


set(libkleo_backend_chiasmus_SRCS 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/config_data.c 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/obtainkeysjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/symcryptrunprocessbase.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/chiasmusjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/chiasmuslibrary.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/chiasmusbackend.cpp
	)


set(libkleo_core_SRCS
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

set(libkleo_ui_SRCS
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

kde4_add_ui_files(libkleo_ui_SRCS
	${CMAKE_SOURCE_DIR}/libkleo/ui/adddirectoryservicedialog.ui
	${CMAKE_SOURCE_DIR}/libkleo/ui/directoryserviceswidgetbase.ui
)

set(kleo_LIB_SRCS ${libkleo_core_SRCS} ${libkleo_ui_SRCS} ${libkleo_backend_chiasmus_SRCS}
   kleo/cryptplug.cpp 
   kleo/cryptplugwrapper.cpp 
   kleo/cryptplugwrapperlist.cpp 
   kleo/cryptplugfactory.cpp )

if(NOT WIN32)
   set(kleo_LIB_SRCS ${kleo_LIB_SRCS} ${libkleo_backend_qgpgme_SRCS})
endif(NOT WIN32)

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

