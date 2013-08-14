project(console)

add_subdirectory(calendarjanitor)

if (KDEPIMLIBS_KRESOURCES_LIBS)
  add_subdirectory(konsolekalendar)
  add_subdirectory(kabcclient)
endif ()

