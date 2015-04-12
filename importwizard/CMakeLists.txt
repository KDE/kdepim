project(importwizard)

include_directories(
		    ${CMAKE_SOURCE_DIR}/mailcommon/filter/
		    ${CMAKE_SOURCE_DIR}/importwizard/autodetect/
)


add_subdirectory(icons)

set(importwizard_SRCS
importwizard_debug.cpp
abstractbase.cpp
importwizardkernel.cpp
selectprogrampage.cpp
importaddressbookpage.cpp
importfilterpage.cpp
importmailpage.cpp
importsettingpage.cpp
importwizard.cpp
importfinishpage.cpp
main.cpp
abstractimporter.cpp
selectcomponentpage.cpp
importfilterinfogui.cpp
abstractsettings.cpp
abstractaddressbook.cpp
abstractcalendar.cpp
importcalendarpage.cpp
importwizardutil.cpp
)

set(importwizard_autodetect_SRCS
autodetect/thunderbird/thunderbirdimportdata.cpp
autodetect/thunderbird/thunderbirdsettings.cpp
autodetect/thunderbird/thunderbirdaddressbook.cpp
autodetect/thunderbird/addressbook/MorkParser.cpp
autodetect/sylpheed/sylpheedimportdata.cpp
autodetect/sylpheed/sylpheedsettings.cpp
autodetect/sylpheed/sylpheedaddressbook.cpp
autodetect/sylpheed/sylpheedsettingsutils.cpp
autodetect/evolutionv3/evolutionv3importdata.cpp
autodetect/evolutionv3/evolutionsettings.cpp
autodetect/evolutionv3/evolutioncalendar.cpp
autodetect/evolutionv3/evolutionaddressbook.cpp
autodetect/evolutionv3/evolutionutil.cpp
autodetect/evolutionv2/evolutionv2importdata.cpp
autodetect/evolutionv1/evolutionv1importdata.cpp
autodetect/opera/operaimportdata.cpp
autodetect/opera/operaaddressbook.cpp
autodetect/opera/operasettings.cpp
autodetect/oe/oeimportdata.cpp
autodetect/mailapp/mailappimportdata.cpp
autodetect/pmail/pmailimportdata.cpp
autodetect/pmail/pmailsettings.cpp
autodetect/thebat/thebatimportdata.cpp
autodetect/balsa/balsaimportdata.cpp
autodetect/balsa/balsasettings.cpp
autodetect/balsa/balsaaddressbook.cpp
autodetect/claws-mail/clawsmailimportdata.cpp
autodetect/claws-mail/clawsmailsettings.cpp
autodetect/claws-mail/clawsmailaddressbook.cpp
autodetect/trojita/trojitasettings.cpp
autodetect/trojita/trojitaimportdata.cpp
autodetect/trojita/trojitaaddressbook.cpp
autodetect/icedove/icedoveimportdata.cpp
)


ki18n_wrap_ui(importwizard_SRCS 
                  ui/selectprogrampage.ui
                  ui/importaddressbookpage.ui
                  ui/importfilterpage.ui
                  ui/importmailpage.ui
                  ui/importsettingpage.ui
                  ui/selectcomponentpage.ui 
		  ui/importfinishpage.ui 
		  ui/importcalendarpage.ui)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-kontact-import-wizard.png")
ecm_add_app_icon(importwizard_SRCS ICONS ${ICONS_SRCS})

add_executable(importwizard ${importwizard_SRCS} ${importwizard_autodetect_SRCS})

target_link_libraries(importwizard
  KF5::KIOCore
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::Mime
  KF5::MailTransport
  KF5::DBusAddons
  mailimporter
  pimcommon
  mailcommon
  kdepim
  messagecomposer
  messageviewer
)

install(TARGETS importwizard  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.importwizard.desktop DESTINATION ${KDE_INSTALL_APPDIR})
