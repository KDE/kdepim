# - Try :to find the assuan v2 library

# Variables set:
#  ASSUAN2_{INCLUDES,FOUND,LIBRARIES} will be set for each of the above

# do away with crappy condition repetition on else/endfoo
set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS_assuan2_saved ${CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS} )
set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true )

#if this is built-in, please replace, if it isn't, export into a MacroToBool.cmake of it's own
macro( macro_bool_to_bool FOUND_VAR )
  foreach( _current_VAR ${ARGN} )
    if ( ${FOUND_VAR} )
      set( ${_current_VAR} TRUE )
    else()
      set( ${_current_VAR} FALSE )
    endif()
  endforeach()
endmacro()

include (MacroEnsureVersion)
include (MacroBoolTo01)
include (MacroLogFeature)

if ( WIN32 )

  # On Windows, we don't have a libassuan-config script, so we need to
  # look for the stuff ourselves:

  # in cmake, AND and OR have the same precedence, there's no
  # subexpressions, and expressions are evaluated short-circuit'ed
  # IOW: CMake if() suxx.
  set( _seem_to_have_cached_assuan2 false )
  if ( ASSUAN2_INCLUDES )
    if ( ASSUAN2_VANILLA_LIBRARIES )#OR ASSUAN2_QT_LIBRARIES OR ASSUAN2_GLIB_LIBRARIES )
      set( _seem_to_have_cached_assuan2 true )
    endif()
  endif()

  if ( _seem_to_have_cached_assuan2 )

    macro_bool_to_bool( ASSUAN2_VANILLA_LIBRARIES  ASSUAN2_VANILLA_FOUND )
    # this would have been preferred:
    #set( ASSUAN2_*_FOUND macro_bool_to_bool(ASSUAN2_*_LIBRARIES) )

    if ( ASSUAN2_VANILLA_FOUND ) #OR ASSUAN2_GLIB_FOUND OR ASSUAN2_QT_FOUND )
      set( ASSUAN2_FOUND true )
    else()
      set( ASSUAN2_FOUND false )
    endif()

  else()

    set( ASSUAN2_FOUND         false )
    set( ASSUAN2_VANILLA_FOUND false )
    #set( ASSUAN2_GLIB_FOUND    false )
    #set( ASSUAN2_QT_FOUND      false )

    find_path( ASSUAN2_INCLUDES assuan.h
      ${CMAKE_INCLUDE_PATH}
      ${CMAKE_INSTALL_PREFIX}/include
    )

    if (NOT WINCE)
      find_library( _assuan2_library NAMES assuan2 libassuan2 assuan-0 libassuan-0 #sic!
        PATHS 
          ${CMAKE_LIBRARY_PATH}
          ${CMAKE_INSTALL_PREFIX}/lib
      )
    else (NOT WINCE)
      find_library( _assuan2_library NAMES libassuan-0-msc
        PATHS 
          ${CMAKE_LIBRARY_PATH}
          ${CMAKE_INSTALL_PREFIX}/lib
      )
    endif (NOT WINCE)

    if (NOT WINCE)
      find_library( _gpg_error_library     NAMES gpg-error libgpg-error gpg-error-0 libgpg-error-0
        PATHS 
          ${CMAKE_LIBRARY_PATH}
          ${CMAKE_INSTALL_PREFIX}/lib
      )
    else (NOT WINCE)
      find_library( _gpg_error_library     NAMES libgpg-error-0-msc
        PATHS 
          ${CMAKE_LIBRARY_PATH}
          ${CMAKE_INSTALL_PREFIX}/lib
      )
    endif (NOT WINCE)

    set( ASSUAN2_INCLUDES ${ASSUAN2_INCLUDES} )

    if ( _assuan2_library AND _gpg_error_library )
      set( ASSUAN2_LIBRARIES ${_assuan2_library} ${_gpg_error_library} ws2_32 )
      set( ASSUAN2_FOUND             true )
    endif()

  endif()

  macro_bool_to_01( ASSUAN2_FOUND HAVE_ASSUAN2 )

else() # not WIN32

  # On *nix, we have the libassuan-config script which can tell us all we
  # need to know:

  # see WIN32 case for an explanation of what this does:
  set( _seem_to_have_cached_assuan2 false )
  if ( ASSUAN2_INCLUDES AND ASSUAN2_LIBRARIES )
    set( _seem_to_have_cached_assuan2 true )
  endif()

  if ( _seem_to_have_cached_assuan2 )

    set( ASSUAN2_FOUND true )

  else()

    set( ASSUAN2_FOUND         false )

    find_program( _ASSUAN2CONFIG_EXECUTABLE NAMES libassuan-config )

    # if libassuan-config has been found
    if ( _ASSUAN2CONFIG_EXECUTABLE )
      
      message( STATUS "Found libassuan-config at ${_ASSUAN2CONFIG_EXECUTABLE}" )

      exec_program( ${_ASSUAN2CONFIG_EXECUTABLE} ARGS --version OUTPUT_VARIABLE ASSUAN2_VERSION )

      set( _ASSUAN2_MIN_VERSION "2.0.0" )
      macro_ensure_version( ${_ASSUAN2_MIN_VERSION} ${ASSUAN2_VERSION} _ASSUAN2_INSTALLED_VERSION_OK )

      if ( NOT _ASSUAN2_INSTALLED_VERSION_OK )

        message( STATUS "The installed version of assuan is too old: ${ASSUAN2_VERSION} (required: >= ${_ASSUAN2_MIN_VERSION})" )

      else()

        message( STATUS "Found assuan v${ASSUAN2_VERSION}" )

        exec_program( ${_ASSUAN2CONFIG_EXECUTABLE} ARGS --libs OUTPUT_VARIABLE _assuan2_config_libs RETURN_VALUE _ret )
	if ( _ret )
	  set( _assuan2_config_libs )
	endif()

        # append -lgpg-error to the list of libraries, if necessary
        if ( _assuan2_config_libs AND NOT _assuan2_config_libs MATCHES "lgpg-error" )
          set( _assuan2_config_libs "${_assuan2_config_libs} -lgpg-error" )
        endif()

        if ( _assuan2_config_libs )

          exec_program( ${_ASSUAN2CONFIG_EXECUTABLE} ARGS --cflags OUTPUT_VARIABLE _ASSUAN2_CFLAGS )

          if ( _ASSUAN2_CFLAGS )
            string( REGEX REPLACE "(\r?\n)+$" " " _ASSUAN2_CFLAGS  "${_ASSUAN2_CFLAGS}" )
            string( REGEX REPLACE " *-I"      ";" ASSUAN2_INCLUDES "${_ASSUAN2_CFLAGS}" )
          endif()

          if ( _assuan2_config_libs )

            set( _assuan2_library_dirs )
            set( _assuan2_library_names )

            string( REGEX REPLACE " +" ";" _assuan2_config_libs "${_assuan2_config_libs}" )

            foreach( _flag ${_assuan2_config_libs} )
              if ( "${_flag}" MATCHES "^-L" )
                string( REGEX REPLACE "^-L" "" _dir "${_flag}" )
                file( TO_CMAKE_PATH "${_dir}" _dir )
                set( _assuan2_library_dirs ${_assuan2_library_dirs} "${_dir}" )
              elseif( "${_flag}" MATCHES "^-l" )
                string( REGEX REPLACE "^-l" "" _name "${_flag}" )
                set( _assuan2_library_names ${_assuan2_library_names} "${_name}" )
              endif()
            endforeach()

            set( ASSUAN2_FOUND true )

            foreach( _name ${_assuan2_library_names} )
              set( _assuan2_${_name}_lib )

              # if -L options were given, look only there
              if ( _assuan2_library_dirs )
                find_library( _assuan2_${_name}_lib NAMES ${_name} PATHS ${_assuan2_library_dirs} NO_DEFAULT_PATH )
              endif()

              # if not found there, look in system directories
              if ( NOT _assuan2_${_name}_lib )
                find_library( _assuan2_${_name}_lib NAMES ${_name} )
              endif()

              # if still not found, then the whole flavour isn't found
              if ( NOT _assuan2_${_name}_lib )
                if ( ASSUAN2_FOUND )
                  set( ASSUAN2_FOUND false )
                  set( _not_found_reason "dependant library ${_name} wasn't found" )
                endif()
              endif()

              set( ASSUAN2_LIBRARIES ${ASSUAN2_LIBRARIES} "${_assuan2_${_name}_lib}" )
            endforeach()

            #check_c_library_exists_explicit( assuan         assuan_check_version "${_ASSUAN2_CFLAGS}" "${ASSUAN2_LIBRARIES}"         ASSUAN2_FOUND         )
            if ( ASSUAN2_FOUND )
              message( STATUS " Checking whether assuan is usable...yes" )
            else()
              message( STATUS " Checking whether assuan is usable...no" )
              message( STATUS "  (${_not_found_reason})" )
            endif()
          endif()

          # ensure that they are cached
          set( ASSUAN2_INCLUDES  ${ASSUAN2_INCLUDES}  )
          set( ASSUAN2_LIBRARIES ${ASSUAN2_LIBRARIES} )

        endif()

      endif()

    endif()

  endif()

  macro_bool_to_01( ASSUAN2_FOUND         HAVE_ASSUAN2         )

endif() # WIN32 | Unix


if ( NOT Assuan2_FIND_QUIETLY )

  if ( ASSUAN2_FOUND )
    message( STATUS "Usable assuan found." )
    message( STATUS " Includes:  ${ASSUAN2_INCLUDES}" )
    message( STATUS " Libraries: ${ASSUAN2_LIBRARIES}" )
  else()
    message( STATUS "No usable assuan found." )
  endif()

  macro_bool_to_bool( Assuan2_FIND_REQUIRED _req )

  if ( WIN32 )
    set( _assuan2_homepage "http://www.gpg4win.org" )
  else()
    set( _assuan2_homepage "http://www.gnupg.org/related_software/libassuan" )
  endif()

  macro_log_feature(
    ASSUAN2_FOUND
    "assuan2"
    "Assuan v2 IPC library"
    ${_assuan2_homepage}
    ${_req}
    "${_ASSUAN2_MIN_VERSION} or greater"
    "Needed for Kleopatra to act as the GnuPG UI Server"
  )

else()

  if ( Assuan2_FIND_REQUIRED AND NOT ASSUAN2_FOUND )
    message( FATAL_ERROR "" )
  endif()

endif()

set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS_assuan2_saved )
