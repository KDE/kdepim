if(BUILD_akregator)
  add_subdirectory(akregator)
endif(BUILD_akregator)

if(BUILD_kabcclient)
  add_subdirectory(kabcclient)
endif(BUILD_kabcclient)

if(BUILD_kalarm)
  add_subdirectory(kalarm)
endif(BUILD_kalarm)

if(BUILD_kioslave)
  add_subdirectory(kioslave)
endif(BUILD_kioslave)

if(BUILD_kjots)
  add_subdirectory(kjots)
endif(BUILD_kjots)

if(BUILD_kleopatra)
  add_subdirectory(kleopatra)
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

if(BUILD_konsolekalendar)
  add_subdirectory(konsolekalendar)
endif(BUILD_konsolekalendar)

if(BUILD_kontact)
  add_subdirectory(kontact)
  add_subdirectory(kontact-admin)
endif(BUILD_kontact)

if(BUILD_kpilot)
  add_subdirectory(kpilot)
endif(BUILD_kpilot)

if(BUILD_ktimetracker)
  add_subdirectory(ktimetracker)
endif(BUILD_ktimetracker)

#if(BUILD_kwatchgnupg)
#  add_subdirectory(kwatchgnupg)
#endif(BUILD_kwatchgnupg)
