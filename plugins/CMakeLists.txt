if(MESSAGEVIEWER_SUPPORTED)
  add_subdirectory(messageviewer)
endif()

if(KDEPIM_BUILD_DESKTOP)
  add_subdirectory(ktexteditor)
endif()

add_subdirectory(kaddressbook)
