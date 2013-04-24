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

if(BUILD_archivemailagent)
  add_subdirectory(archivemailagent)
endif()

