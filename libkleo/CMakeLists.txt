include_directories( ${CMAKE_BINARY_DIR}/kleo/backends/chiasmus/ ${GPGME_INCLUDES} )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

add_subdirectory( pics )
add_subdirectory( backends )
add_subdirectory( tests )


########### next target ###############
set(libkleo_backend_qgpgme_SRCS
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/gnupgprocessbase.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmeprogresstokenmapper.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmebackend.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmejob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmekeylistjob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmekeygenerationjob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmeimportjob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmeexportjob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmesecretkeyexportjob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmedownloadjob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmedeletejob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmeencryptjob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmedecryptjob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmesignjob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmeverifydetachedjob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmeverifyopaquejob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmesignencryptjob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmedecryptverifyjob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmecryptoconfig.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/qgpgme/qgpgmerefreshkeysjob.cpp
	)


set(libkleo_backend_chiasmus_SRCS 
	${CMAKE_SOURCE_DIR}/kleo/backends/chiasmus/config_data.c 
	${CMAKE_SOURCE_DIR}/kleo/backends/chiasmus/obtainkeysjob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/chiasmus/symcryptrunprocessbase.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/chiasmus/chiasmusjob.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/chiasmus/chiasmuslibrary.cpp 
	${CMAKE_SOURCE_DIR}/kleo/backends/chiasmus/chiasmusbackend.cpp
	)


set(libkleo_core_SRCS
   ${CMAKE_SOURCE_DIR}/kleo/kleo/cryptobackend.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/kleo/cryptobackendfactory.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/kleo/enum.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/kleo/dn.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/kleo/job.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/kleo/multideletejob.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/kleo/hierarchicalkeylistjob.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/kleo/kconfigbasedkeyfilter.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/kleo/keyfiltermanager.cpp
   )

set(libkleo_ui_SRCS
   ${CMAKE_SOURCE_DIR}/kleo/ui/kdhorizontalline.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/ui/progressbar.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/ui/progressdialog.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/ui/keylistview.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/ui/keyselectiondialog.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/ui/keyrequester.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/ui/keyapprovaldialog.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/ui/backendconfigwidget.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/ui/dnattributeorderconfigwidget.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/ui/cryptoconfigmodule.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/ui/cryptoconfigdialog.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/ui/directoryserviceswidget.cpp 
   ${CMAKE_SOURCE_DIR}/kleo/ui/adddirectoryservicedialogimpl.cpp
)

kde4_add_ui_files(libkleo_ui_SRCS
	${CMAKE_SOURCE_DIR}/kleo/ui/adddirectoryservicedialog.ui
	${CMAKE_SOURCE_DIR}/kleo/ui/directoryserviceswidgetbase.ui
)

set(kleo_LIB_SRCS ${libkleo_core_SRCS} ${libkleo_ui_SRCS} ${libkleo_backend_chiasmus_SRCS}
   cryptplug.cpp 
   cryptplugwrapper.cpp 
   cryptplugwrapperlist.cpp 
   cryptplugfactory.cpp )

if(NOT WIN32)
   set(kleo_LIB_SRCS ${kleo_LIB_SRCS} ${libkleo_backend_qgpgme_SRCS})
endif(NOT WIN32)

kde4_add_library(kleo-gpl SHARED ${kleo_LIB_SRCS})

target_link_libraries(kleo-gpl ${KDE4_KDE3SUPPORT_LIBS} gpgme qgpgme ${GPGME_LIBRARIES} ${QT_QT3SUPPORT_LIBRARY})

set_target_properties(kleo-gpl PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kleo-gpl  DESTINATION ${LIB_INSTALL_DIR})

install( FILES 
	cryptplug.h
	cryptplugfactory.h
	cryptplugwrapper.h
	cryptplugwrapperlist.h
	kleo_export.h
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

