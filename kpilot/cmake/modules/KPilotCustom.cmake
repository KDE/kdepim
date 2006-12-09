MACRO(KDE3_INSTALL_ICONS_CUSTOM _theme)
   ADD_CUSTOM_TARGET(install_icons )
   SET_TARGET_PROPERTIES(install_icons PROPERTIES POST_INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake )
   FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake "# icon installations rules\n")
   FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake "SET(CMAKE_BACKWARDS_COMPATIBILITY \"2.2\") \n")

   FILE(GLOB _icons *.png)
   FOREACH(_current_ICON ${_icons} )
      STRING(REGEX REPLACE "^.*/[a-zA-Z]+([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" "\\1" _size "${_current_ICON}")
      STRING(REGEX REPLACE "^.*/[a-zA-Z]+([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" "\\2" _group "${_current_ICON}")
      STRING(REGEX REPLACE "^.*/[a-zA-Z]+([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" "\\3" _name "${_current_ICON}")

      SET(_icon_GROUP "unknown")

      IF(${_group} STREQUAL "mime")
         SET(_icon_GROUP  "mimetypes")
      ENDIF(${_group} STREQUAL "mime")

      IF(${_group} STREQUAL "filesys")
         SET(_icon_GROUP  "filesystems")
      ENDIF(${_group} STREQUAL "filesys")

      IF(${_group} STREQUAL "device")
         SET(_icon_GROUP  "devices")
      ENDIF(${_group} STREQUAL "device")

      IF(${_group} STREQUAL "app")
         SET(_icon_GROUP  "apps")
      ENDIF(${_group} STREQUAL "app")

      IF(${_group} STREQUAL "action")
         SET(_icon_GROUP  "actions")
      ENDIF(${_group} STREQUAL "action")

      IF( NOT ${_icon_GROUP} STREQUAL "unknown")
#        message(STATUS "icon: ${_current_ICON} size: ${_size} group: ${_group} name: ${_name}" )
         SET(_ICON_INSTALL_NAME ${CMAKE_INSTALL_PREFIX}/share/icons/${_theme}/${_size}x${_size}/${_icon_GROUP}/${_name})
         FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake "message(STATUS \"Installing ${_ICON_INSTALL_NAME}\") \n")
         FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake "CONFIGURE_FILE( ${_current_ICON} ${_ICON_INSTALL_NAME} COPYONLY) \n")
      ELSE( NOT ${_icon_GROUP} STREQUAL "unknown")
         message(STATUS "icon: ${_current_ICON} doesn't fit naming conventions. ignoring." )
      ENDIF( NOT ${_icon_GROUP} STREQUAL "unknown")

   ENDFOREACH (_current_ICON)
ENDMACRO(KDE3_INSTALL_ICONS_CUSTOM)


MACRO(KPILOT_RPATH _thing)
	set_target_properties(${_thing} PROPERTIES 
		INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib;${KDE3_DIR}/lib;${PILOTLINK_LIBRARY}
		INSTALL_RPATH_USE_LINK_PATH true
	)
ENDMACRO(KPILOT_RPATH _thing)
