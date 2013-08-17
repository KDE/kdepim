project(console)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(calendarjanitor)

if (KDEPIMLIBS_KRESOURCES_LIBS)
  add_subdirectory(konsolekalendar)
  add_subdirectory(kabcclient)
endif ()

