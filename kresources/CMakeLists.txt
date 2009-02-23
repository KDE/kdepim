# taken from Akonadi which took it from FindQt4.cmake to add additional includes, should probably be merged back there
MACRO(QT4_ADD_DBUS_INTERFACE2 _sources _interface _basename _include)
  GET_FILENAME_COMPONENT(_infile ${_interface} ABSOLUTE)
  SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
  SET(_impl   ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
  SET(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)
  SET(_params -m -i ${_include} -p)

  ADD_CUSTOM_COMMAND(OUTPUT ${_impl} ${_header}
      COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} ${_params} ${_basename} ${_infile}
      DEPENDS ${_infile})

  SET_SOURCE_FILES_PROPERTIES(${_impl} PROPERTIES SKIP_AUTOMOC TRUE)

  QT4_GENERATE_MOC(${_header} ${_moc})

  SET(${_sources} ${${_sources}} ${_impl} ${_header} ${_moc})
  MACRO_ADD_FILE_DEPENDENCIES(${_impl} ${_moc})
ENDMACRO(QT4_ADD_DBUS_INTERFACE2)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory( lib ) 
add_subdirectory( birthdays ) 
add_subdirectory( blog ) 
add_subdirectory( remote ) 
add_subdirectory( kolab ) 
#add_subdirectory( featureplan ) 
# FIXME disabled for the moment, since it is not needed
#add_subdirectory( akonadi )

if(BUILD_EVERYTHING)
  add_subdirectory( egroupware ) 
  add_subdirectory( scalix ) 
  add_subdirectory( slox ) 
  add_subdirectory( groupdav ) 
  add_subdirectory( groupwise )
endif(BUILD_EVERYTHING)
