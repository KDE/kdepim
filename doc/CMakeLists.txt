if(BUILD_akregator)
  add_subdirectory(akregator)
endif(BUILD_akregator)

if(BUILD_blogilo)
  add_subdirectory(blogilo)
endif(BUILD_blogilo)

if(BUILD_console)
  add_subdirectory(kabcclient)
  add_subdirectory(konsolekalendar)
endif(BUILD_console)

if(BUILD_kalarm)
  add_subdirectory(kalarm)
endif(BUILD_kalarm)

add_subdirectory(kioslave)

if(BUILD_kjots)
  add_subdirectory(kjots)
endif(BUILD_kjots)

if(BUILD_kleopatra)
  add_subdirectory(kleopatra)
  add_subdirectory(kwatchgnupg)
endif(BUILD_kleopatra)

if(BUILD_kmail)
  add_subdirectory(kmail)
  if(BUILD_korganizer)
    add_subdirectory(korganizer)
  endif(BUILD_korganizer)
endif(BUILD_kmail)

if(BUILD_knode)
  add_subdirectory(knode)
endif(BUILD_knode)

if(BUILD_knotes)
  add_subdirectory(knotes)
endif(BUILD_knotes)

if(BUILD_kontact)
  add_subdirectory(kontact)
  add_subdirectory(kontact-admin)
endif(BUILD_kontact)

if(BUILD_ktimetracker)
  add_subdirectory(ktimetracker)
endif(BUILD_ktimetracker)
