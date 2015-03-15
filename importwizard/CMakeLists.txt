project(importwizard)


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
thunderbird/thunderbirdimportdata.cpp
thunderbird/thunderbirdsettings.cpp
thunderbird/thunderbirdaddressbook.cpp
thunderbird/addressbook/MorkParser.cpp
sylpheed/sylpheedimportdata.cpp
sylpheed/sylpheedsettings.cpp
sylpheed/sylpheedaddressbook.cpp
sylpheed/sylpheedsettingsutils.cpp
evolutionv3/evolutionv3importdata.cpp
evolutionv3/evolutionsettings.cpp
evolutionv3/evolutioncalendar.cpp
evolutionv3/evolutionaddressbook.cpp
evolutionv3/evolutionutil.cpp
evolutionv2/evolutionv2importdata.cpp
evolutionv1/evolutionv1importdata.cpp
opera/operaimportdata.cpp
opera/operaaddressbook.cpp
opera/operasettings.cpp
oe/oeimportdata.cpp
mailapp/mailappimportdata.cpp
pmail/pmailimportdata.cpp
pmail/pmailsettings.cpp
thebat/thebatimportdata.cpp
balsa/balsaimportdata.cpp
balsa/balsasettings.cpp
balsa/balsaaddressbook.cpp
claws-mail/clawsmailimportdata.cpp
claws-mail/clawsmailsettings.cpp
claws-mail/clawsmailaddressbook.cpp
trojita/trojitasettings.cpp
trojita/trojitaimportdata.cpp
trojita/trojitaaddressbook.cpp
icedove/icedoveimportdata.cpp
)

include_directories(
		    ${CMAKE_SOURCE_DIR}/mailcommon/filter/
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

add_executable(importwizard ${importwizard_SRCS})

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
