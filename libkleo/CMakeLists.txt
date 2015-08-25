project(libkleo)

# target_include_directories does not handle empty include paths
include_directories(${GPGME_INCLUDES})

if (KDEPIM_ONLY_KLEO)
  add_definitions( -DKDEPIM_ONLY_KLEO )
endif ()

add_definitions(-DTRANSLATION_DOMAIN=\"libkleopatra\")

#add_definitions( -DQT_NO_CAST_FROM_ASCII )
#add_definitions( -DQT_NO_CAST_TO_ASCII )

kde_enable_exceptions()

add_definitions( -DGPGMEPP_ERR_SOURCE_DEFAULT=13 ) # 13 is GPG_ERR_SOURCE_KLEO, even if gpg-error's too old to know about
#parts of Boost don't build with the gcc pedantic option, so remove that option
string(REPLACE "-pedantic" "" FOO "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${FOO}")

add_subdirectory( pics )
add_subdirectory( backends )
if (BUILD_TESTING)
   add_subdirectory( tests )
endif()

########### next target ###############
set(libkleo_backend_qgpgme_SRCS
    backends/qgpgme/gnupgprocessbase.cpp
    backends/qgpgme/qgpgmeprogresstokenmapper.cpp
    backends/qgpgme/qgpgmebackend.cpp
    backends/qgpgme/threadedjobmixin.cpp
    backends/qgpgme/qgpgmekeylistjob.cpp
    backends/qgpgme/qgpgmelistallkeysjob.cpp
    backends/qgpgme/qgpgmekeygenerationjob.cpp
    backends/qgpgme/qgpgmeimportjob.cpp
    backends/qgpgme/qgpgmeimportfromkeyserverjob.cpp
    backends/qgpgme/qgpgmeexportjob.cpp
    backends/qgpgme/qgpgmesecretkeyexportjob.cpp
    backends/qgpgme/qgpgmechangeexpiryjob.cpp
    backends/qgpgme/qgpgmechangeownertrustjob.cpp
    backends/qgpgme/qgpgmechangepasswdjob.cpp
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
    backends/qgpgme/qgpgmenewcryptoconfig.cpp
    backends/qgpgme/qgpgmerefreshkeysjob.cpp
    )
ecm_qt_declare_logging_category(libkleo_backend_qgpgme_SRCS HEADER gpgme_backend_debug.h IDENTIFIER GPGPME_BACKEND_LOG CATEGORY_NAME log_gpgme_backend)

set(libkleo_backend_chiasmus_SRCS
    backends/chiasmus/config_data.c
    backends/chiasmus/obtainkeysjob.cpp
    backends/chiasmus/symcryptrunprocessbase.cpp
    backends/chiasmus/chiasmusjob.cpp
    backends/chiasmus/chiasmuslibrary.cpp
    backends/chiasmus/chiasmusbackend.cpp
    )


set(libkleo_core_SRCS
   kleo/checksumdefinition.cpp
   kleo/cryptobackend.cpp
   kleo/cryptobackendfactory.cpp
   kleo/enum.cpp
   kleo/exception.cpp
   kleo/dn.cpp
   kleo/job.cpp
   kleo/multideletejob.cpp
   kleo/hierarchicalkeylistjob.cpp
   kleo/kconfigbasedkeyfilter.cpp
   kleo/keyfiltermanager.cpp
   )

set(libkleo_ui_common_SRCS
   ui/dnattributeorderconfigwidget.cpp
   ui/kdhorizontalline.cpp
   ui/filenamerequester.cpp
   ui/messagebox.cpp
   ui/cryptoconfigmodule.cpp
   ui/cryptoconfigdialog.cpp
   ui/directoryserviceswidget.cpp
   ui/progressbar.cpp
   ui/progressdialog.cpp
   ui/auditlogviewer.cpp
   )
ecm_qt_declare_logging_category(libkleo_ui_common_SRCS HEADER kleo_ui_debug.h IDENTIFIER KLEO_UI_LOG CATEGORY_NAME log_kleo_ui)

set(libkleo_ui_SRCS    # make this a separate lib.
    ui/keylistview.cpp
    ui/keyselectiondialog.cpp
    ui/keyrequester.cpp
    ui/keyapprovaldialog.cpp
    )

ki18n_wrap_ui(libkleo_ui_common_SRCS
    ui/directoryserviceswidget.ui
)

set(kleo_LIB_SRCS ${libkleo_core_SRCS} ${libkleo_ui_common_SRCS} ${libkleo_backend_qgpgme_SRCS})
set(kleo_LIB_LIBS PUBLIC KF5::QGpgme PRIVATE pimcommon)
if ( NOT KDEPIM_ONLY_KLEO )
   set( kleo_LIB_SRCS ${kleo_LIB_SRCS} ${libkleo_backend_chiasmus_SRCS} ${libkleo_ui_SRCS})
   set( kleo_LIB_LIBS ${kleo_LIB_LIBS} PUBLIC KF5::Gpgmepp PRIVATE KF5::WidgetsAddons KF5::Completion KF5::I18n KF5::IconThemes KF5::KDELibs4Support)
endif ()

add_library(kleo ${kleo_LIB_SRCS})
generate_export_header(kleo BASE_NAME kleo)
if(WIN32)
target_link_libraries(kleo ${kleo_LIB_LIBS}  ${GPGME_VANILLA_LIBRARIES} )
else()
target_link_libraries(kleo ${kleo_LIB_LIBS} )
endif()
set_target_properties(kleo PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION} )
install(TARGETS kleo ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

target_include_directories(kleo PUBLIC "$<BUILD_INTERFACE:${libkleo_SOURCE_DIR};${libkleo_BINARY_DIR};${Boost_INCLUDE_DIR}>")

if ( WIN32 )
install ( FILES libkleopatrarc-win32.desktop DESTINATION ${KDE_INSTALL_CONFDIR} RENAME libkleopatrarc )
else ()
install ( FILES libkleopatrarc.desktop DESTINATION ${KDE_INSTALL_CONFDIR} RENAME libkleopatrarc )
endif ()
