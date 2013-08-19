# - Try to find the assuan library
#
# Algorithm:
#  - Windows:
#    On Windows, there's three assuan variants: assuan{,-glib,-qt}.
#    - The variant used determines the event loop integration possible:
#      - assuan:      no event loop integration possible, only synchronous operations supported
#      - assuan-glib: glib event loop integration possible, only asynchronous operations supported
#      - assuan-qt:   qt event loop integration possible, only asynchronous operations supported
#    - ASSUAN_{VANILLA,GLIB,QT}_{FOUND,LIBRARIES} will be set for each of the above
#    - ASSUAN_INCLUDES is the same for all of the above
#    - ASSUAN_FOUND is set if any of the above was found
#  - *nix:
#    There's also three variants: assuan{,-pthread,-pth}.
#    - The variant used determines the multithreaded use possible:
#      - assuan:         no multithreading support available
#      - assuan-pthread: multithreading available using POSIX threads
#      - assuan-pth:     multithreading available using GNU PTH (cooperative multithreading)
#    - ASSUAN_{VANILLA,PTH,PTHREAD}_{FOUND,LIBRARIES} will be set for each of the above
#    - ASSUAN_INCLUDES is the same for all of the above
#    - ASSUAN_FOUND is set if any of the above was found
#

#
# THIS IS ALMOST A 1:1 COPY OF FindGpgme.cmake.
# Any changes to here likely apply there, too.
# Therefore, the as-yet-nonexistant -glib nd -qt flavours are only commented out, not removed.
#

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

if ( WIN32 )

  # On Windows, we don't have a libassuan-config script, so we need to
  # look for the stuff ourselves:

  # in cmake, AND and OR have the same precedence, there's no
  # subexpressions, and expressions are evaluated short-circuit'ed
  # IOW: CMake if() suxx.
  set( _seem_to_have_cached_assuan false )
  if ( ASSUAN_INCLUDES )
    if ( ASSUAN_VANILLA_LIBRARIES )#OR ASSUAN_QT_LIBRARIES OR ASSUAN_GLIB_LIBRARIES )
      set( _seem_to_have_cached_assuan true )
    endif()
  endif()

  if ( _seem_to_have_cached_assuan )

    macro_bool_to_bool( ASSUAN_VANILLA_LIBRARIES  ASSUAN_VANILLA_FOUND )
    # this would have been preferred:
    #set( ASSUAN_*_FOUND macro_bool_to_bool(ASSUAN_*_LIBRARIES) )

    if ( ASSUAN_VANILLA_FOUND ) #OR ASSUAN_GLIB_FOUND OR ASSUAN_QT_FOUND )
      set( ASSUAN_FOUND true )
    else()
      set( ASSUAN_FOUND false )
    endif()

  else()

    set( ASSUAN_FOUND         false )
    set( ASSUAN_VANILLA_FOUND false )
    #set( ASSUAN_GLIB_FOUND    false )
    #set( ASSUAN_QT_FOUND      false )

    find_path( ASSUAN_INCLUDES assuan.h
      ${CMAKE_INCLUDE_PATH}
      ${CMAKE_INSTALL_PREFIX}/include
    )

    find_library( _assuan_vanilla_library NAMES assuan libassuan assuan-11 libassuan-11
      PATHS 
        ${CMAKE_LIBRARY_PATH}
        ${CMAKE_INSTALL_PREFIX}/lib
    )

    #find_library( _assuan_glib_library    NAMES assuan-glib libassuan-glib assuan-glib-11 libassuan-glib-11
    #  PATHS 
    #    ${CMAKE_LIBRARY_PATH}
    #    ${CMAKE_INSTALL_PREFIX}/lib
    #)

    #find_library( _assuan_qt_library      NAMES assuan-qt libassuan-qt assuan-qt-11 libassuan-qt-11
    #  PATHS 
    #    ${CMAKE_LIBRARY_PATH}
    #    ${CMAKE_INSTALL_PREFIX}/lib
    #)

    find_library( _gpg_error_library     NAMES gpg-error libgpg-error gpg-error-0 libgpg-error-0
      PATHS 
        ${CMAKE_LIBRARY_PATH}
        ${CMAKE_INSTALL_PREFIX}/lib
    )

    set( ASSUAN_INCLUDES ${ASSUAN_INCLUDES} )

    if ( _assuan_vanilla_library AND _gpg_error_library )
      set( ASSUAN_VANILLA_LIBRARIES ${_assuan_vanilla_library} ${_gpg_error_library} ws2_32 )
      set( ASSUAN_VANILLA_FOUND     true )
      set( ASSUAN_FOUND             true )
    endif()

    #if ( _assuan_glib_library AND _gpg_error_library )
    #  set( ASSUAN_GLIB_LIBRARIES    ${_assuan_glib_library}    ${_gpg_error_library} )
    #  set( ASSUAN_GLIB_FOUND        true )
    #  set( ASSUAN_FOUND             true )
    #endif()

    #if ( _assuan_qt_library AND _gpg_error_library )
    #  set( ASSUAN_QT_LIBRARIES      ${_assuan_qt_library}      ${_gpg_error_library} )
    #  set( ASSUAN_QT_FOUND          true )
    #  set( ASSUAN_FOUND             true )
    #endif()

  endif()

  # these are Unix-only:
  set( ASSUAN_PTHREAD_FOUND false )
  set( ASSUAN_PTH_FOUND     false )
  set( HAVE_ASSUAN_PTHREAD  0     )
  set( HAVE_ASSUAN_PTH      0     )

  macro_bool_to_01( ASSUAN_FOUND         HAVE_ASSUAN         )
  macro_bool_to_01( ASSUAN_VANILLA_FOUND HAVE_ASSUAN_VANILLA )
  #macro_bool_to_01( ASSUAN_GLIB_FOUND    HAVE_ASSUAN_GLIB    )
  #macro_bool_to_01( ASSUAN_QT_FOUND      HAVE_ASSUAN_QT      )

else() # not WIN32

  # On *nix, we have the libassuan-config script which can tell us all we
  # need to know:

  # see WIN32 case for an explanation of what this does:
  set( _seem_to_have_cached_assuan false )
  if ( ASSUAN_INCLUDES )
    if ( ASSUAN_VANILLA_LIBRARIES OR ASSUAN_PTHREAD_LIBRARIES OR ASSUAN_PTH_LIBRARIES )
      set( _seem_to_have_cached_assuan true )
    endif()
  endif()

  if ( _seem_to_have_cached_assuan )

    macro_bool_to_bool( ASSUAN_VANILLA_LIBRARIES ASSUAN_VANILLA_FOUND )
    macro_bool_to_bool( ASSUAN_PTHREAD_LIBRARIES ASSUAN_PTHREAD_FOUND )
    macro_bool_to_bool( ASSUAN_PTH_LIBRARIES     ASSUAN_PTH_FOUND     )

    if ( ASSUAN_VANILLA_FOUND OR ASSUAN_PTHREAD_FOUND OR ASSUAN_PTH_FOUND )
      set( ASSUAN_FOUND true )
    else()
      set( ASSUAN_FOUND false )
    endif()

  else()

    set( ASSUAN_FOUND         false )
    set( ASSUAN_VANILLA_FOUND false )
    set( ASSUAN_PTHREAD_FOUND false )
    set( ASSUAN_PTH_FOUND     false )

    find_program( _ASSUANCONFIG_EXECUTABLE NAMES libassuan-config )

    # if libassuan-config has been found
    if ( _ASSUANCONFIG_EXECUTABLE )

      message( STATUS "Found libassuan-config at ${_ASSUANCONFIG_EXECUTABLE}" )

      exec_program( ${_ASSUANCONFIG_EXECUTABLE} ARGS --version OUTPUT_VARIABLE ASSUAN_VERSION )

      set( _ASSUAN_MIN_VERSION "1.0.4" )
      if( ASSUAN_VERSION VERSION_GREATER ${_ASSUAN_MIN_VERSION} )
        set( _ASSUAN_INSTALLED_VERSION_OK TRUE )
      endif()

      if ( NOT _ASSUAN_INSTALLED_VERSION_OK )

        message( STATUS "The installed version of assuan is too old: ${ASSUAN_VERSION} (required: >= ${_ASSUAN_MIN_VERSION})" )

      else()

        message( STATUS "Found assuan v${ASSUAN_VERSION}, checking for flavours..." )

        exec_program( ${_ASSUANCONFIG_EXECUTABLE} ARGS                  --libs OUTPUT_VARIABLE _assuan_config_vanilla_libs RETURN_VALUE _ret )
	if ( _ret )
	  set( _assuan_config_vanilla_libs )
	endif()

        exec_program( ${_ASSUANCONFIG_EXECUTABLE} ARGS --thread=pthread --libs OUTPUT_VARIABLE _assuan_config_pthread_libs RETURN_VALUE _ret )
	if ( _ret )
	  set( _assuan_config_pthread_libs )
	endif()

        exec_program( ${_ASSUANCONFIG_EXECUTABLE} ARGS --thread=pth     --libs OUTPUT_VARIABLE _assuan_config_pth_libs     RETURN_VALUE _ret )
	if ( _ret )
	  set( _assuan_config_pth_libs )
	endif()

        # append -lgpg-error to the list of libraries, if necessary
        foreach ( _flavour vanilla pthread pth )
          if ( _assuan_config_${_flavour}_libs AND NOT _assuan_config_${_flavour}_libs MATCHES "lgpg-error" )
            set( _assuan_config_${_flavour}_libs "${_assuan_config_${_flavour}_libs} -lgpg-error" )
          endif()
        endforeach()

        if ( _assuan_config_vanilla_libs OR _assuan_config_pthread_libs OR _assuan_config_pth_libs )

          exec_program( ${_ASSUANCONFIG_EXECUTABLE} ARGS --cflags OUTPUT_VARIABLE _ASSUAN_CFLAGS )

          if ( _ASSUAN_CFLAGS )
            string( REGEX REPLACE "(\r?\n)+$" " " _ASSUAN_CFLAGS  "${_ASSUAN_CFLAGS}" )
            string( REGEX REPLACE " *-I"      ";" ASSUAN_INCLUDES "${_ASSUAN_CFLAGS}" )
          endif()

          foreach ( _flavour vanilla pthread pth )
            if ( _assuan_config_${_flavour}_libs )

              set( _assuan_library_dirs )
              set( _assuan_library_names )
              string( TOUPPER "${_flavour}" _FLAVOUR )

              string( REGEX REPLACE " +" ";" _assuan_config_${_flavour}_libs "${_assuan_config_${_flavour}_libs}" )

              foreach( _flag ${_assuan_config_${_flavour}_libs} )
                if ( "${_flag}" MATCHES "^-L" )
                  string( REGEX REPLACE "^-L" "" _dir "${_flag}" )
                  file( TO_CMAKE_PATH "${_dir}" _dir )
                  set( _assuan_library_dirs ${_assuan_library_dirs} "${_dir}" )
                elseif( "${_flag}" MATCHES "^-l" )
                  string( REGEX REPLACE "^-l" "" _name "${_flag}" )
                  set( _assuan_library_names ${_assuan_library_names} "${_name}" )
                endif()
              endforeach()

              set( ASSUAN_${_FLAVOUR}_FOUND true )

              foreach( _name ${_assuan_library_names} )
                set( _assuan_${_name}_lib )

                # if -L options were given, look only there
                if ( _assuan_library_dirs )
                  find_library( _assuan_${_name}_lib NAMES ${_name} PATHS ${_assuan_library_dirs} NO_DEFAULT_PATH )
                endif()

                # if not found there, look in system directories
                if ( NOT _assuan_${_name}_lib )
                  find_library( _assuan_${_name}_lib NAMES ${_name} )
                endif()

                # if still not found, then the whole flavour isn't found
                if ( NOT _assuan_${_name}_lib )
                  if ( ASSUAN_${_FLAVOUR}_FOUND )
                    set( ASSUAN_${_FLAVOUR}_FOUND false )
                    set( _not_found_reason "dependant library ${_name} wasn't found" )
                  endif()
                endif()

                set( ASSUAN_${_FLAVOUR}_LIBRARIES ${ASSUAN_${_FLAVOUR}_LIBRARIES} "${_assuan_${_name}_lib}" )
              endforeach()

              #check_c_library_exists_explicit( assuan         assuan_check_version "${_ASSUAN_CFLAGS}" "${ASSUAN_LIBRARIES}"         ASSUAN_FOUND         )
              if ( ASSUAN_${_FLAVOUR}_FOUND )
                message( STATUS " Found flavour '${_flavour}', checking whether it's usable...yes" )
              else()
                message( STATUS " Found flavour '${_flavour}', checking whether it's usable...no" )
                message( STATUS "  (${_not_found_reason})" )
              endif()
            endif()

          endforeach( _flavour )

          # ensure that they are cached
          set( ASSUAN_INCLUDES          ${ASSUAN_INCLUDES}          )
          set( ASSUAN_VANILLA_LIBRARIES ${ASSUAN_VANILLA_LIBRARIES} )
          set( ASSUAN_PTHREAD_LIBRARIES ${ASSUAN_PTHREAD_LIBRARIES} )
          set( ASSUAN_PTH_LIBRARIES     ${ASSUAN_PTH_LIBRARIES}     )

          if ( ASSUAN_VANILLA_FOUND OR ASSUAN_PTHREAD_FOUND OR ASSUAN_PTH_FOUND )
            set( ASSUAN_FOUND true )
          else()
            set( ASSUAN_FOUND false )
          endif()

        endif()

      endif()

    endif()

  endif()

  # these are Windows-only:
  #set( ASSUAN_GLIB_FOUND false )
  #set( ASSUAN_QT_FOUND   false )
  #set( HAVE_ASSUAN_GLIB  0     )
  #set( HAVE_ASSUAN_QT    0     )

  macro_bool_to_01( ASSUAN_FOUND         HAVE_ASSUAN         )
  macro_bool_to_01( ASSUAN_VANILLA_FOUND HAVE_ASSUAN_VANILLA )
  macro_bool_to_01( ASSUAN_PTHREAD_FOUND HAVE_ASSUAN_PTHREAD )
  macro_bool_to_01( ASSUAN_PTH_FOUND     HAVE_ASSUAN_PTH     )

endif() # WIN32 | Unix


set( _assuan_flavours "" )

if ( ASSUAN_VANILLA_FOUND )
  set( _assuan_flavours "${_assuan_flavours} vanilla" )
endif()

#if ( ASSUAN_GLIB_FOUND )
#  set( _assuan_flavours "${_assuan_flavours} Glib" )
#endif()

#if ( ASSUAN_QT_FOUND )
#  set( _assuan_flavours "${_assuan_flavours} Qt" )
#endif()

if ( ASSUAN_PTHREAD_FOUND )
  set( _assuan_flavours "${_assuan_flavours} pthread" )
endif()

if ( ASSUAN_PTH_FOUND )
  set( _assuan_flavours "${_assuan_flavours} pth" )
endif()


if ( NOT Assuan_FIND_QUIETLY )

  if ( ASSUAN_FOUND )
    message( STATUS "Usable assuan flavours found: ${_assuan_flavours}" )
  else()
    message( STATUS "No usable assuan flavours found." )
  endif()

  if( Assuan_FIND_REQUIRED )
    set( _ASSUAN_TYPE "REQUIRED" )
  else()
    set( _ASSUAN_TYPE "OPTIONAL" )
  endif()

  if ( WIN32 )
    set( _assuan_homepage "http://www.gpg4win.org" )
  else()
    set( _assuan_homepage "http://www.gnupg.org/related_software/libassuan" )
  endif()

  set_package_properties(ASSUAN PROPERTIES DESCRIPTION "Assuan IPC library"
                         URL ${_assuan_homepage}
                         TYPE ${_ASSUAN_TYPE}
                         PURPOSE "Needed for Kleopatra to act as the GnuPG UI Server"
  )
else()

  if ( Assuan_FIND_REQUIRED AND NOT ASSUAN_FOUND )
    message( FATAL_ERROR "Assuan is required but was not found." )
  endif()

endif()
