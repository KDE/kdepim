if(BUILD_akregator)
  add_subdirectory(akregator)
endif()

if(BUILD_blogilo)
  add_subdirectory(blogilo)
endif()

if(BUILD_console)
  add_subdirectory(kabcclient)
  add_subdirectory(konsolekalendar)
endif()

if(BUILD_kalarm)
  add_subdirectory(kalarm)
endif()

add_subdirectory(kioslave)

if (BUILD_kjots)
  add_subdirectory(kjots)
endif ()

if(BUILD_kleopatra)
  add_subdirectory(kleopatra)
  add_subdirectory(kwatchgnupg)
endif()

if(BUILD_kmail)
  add_subdirectory(kmail)
  if(BUILD_korganizer)
    add_subdirectory(korganizer)
  endif()
endif()

if(BUILD_knode)
  add_subdirectory(knode)
endif()

if(BUILD_knotes)
  add_subdirectory(knotes)
endif()

if(BUILD_kontact)
  add_subdirectory(kontact)
  add_subdirectory(kontact-admin)
endif()

if(BUILD_ktimetracker)
  add_subdirectory(ktimetracker)
endif()

if(BUILD_ktnef)
  add_subdirectory(ktnef)
endif()

if(BUILD_headerthemeeditor)
  add_subdirectory(headerthemeeditor)
endif()

if(BUILD_agents)
  add_subdirectory(akonadi_archivemail_agent)
  add_subdirectory(akonadi_sendlater_agent)
  add_subdirectory(akonadi_folderarchive_agent)
endif()

if(BUILD_importwizard)
  add_subdirectory(importwizard)
endif()

if(BUILD_pimsettingexporter)
  add_subdirectory(pimsettingexporter)
endif()

if(BUILD_kmailcvt)
  add_subdirectory(kmailcvt)
endif()

if(BUILD_sieveeditor)
  add_subdirectory(sieveeditor)
endif()

if(BUILD_contactthemeeditor)
  add_subdirectory(contactthemeeditor)
endif()
