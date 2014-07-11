# Copyright (c) 2013 Sandro Knauß <mail@sandroknauss.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set( GNUPGHOME ${CMAKE_BINARY_DIR}/messagecore/tests/gnupg_home )
add_definitions( -DGNUPGHOME="\\"${GNUPGHOME}\\"" )

macro (KDE4_HANDLE_CRYPTO_RPATH_FOR_EXECUTABLE _target_NAME)
   if (UNIX)
      if (APPLE)
         set(_library_path_variable "DYLD_LIBRARY_PATH")
      elseif (CYGWIN)
         set(_library_path_variable "PATH")
      else (APPLE)
         set(_library_path_variable "LD_LIBRARY_PATH")
      endif (APPLE)

      if (APPLE)
         # DYLD_LIBRARY_PATH does not work like LD_LIBRARY_PATH
         # OSX already has the RPATH in libraries and executables, putting runtime directories in
         # DYLD_LIBRARY_PATH actually breaks things
         set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/:${KDE4_LIB_DIR}")
      else (APPLE)
         set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/:${LIB_INSTALL_DIR}:${KDE4_LIB_DIR}:${QT_LIBRARY_DIR}")
      endif (APPLE)
      get_target_property(_executable ${_target_NAME} LOCATION )

      # use add_custom_target() to have the sh-wrapper generated during build time instead of cmake time
      if (CMAKE_VERSION VERSION_GREATER 2.8.4)
         add_custom_command(TARGET ${_target_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            -D_filename=${_executable}.shell -D_library_path_variable=${_library_path_variable}
            -D_ld_library_path="${_ld_library_path}" -D_executable=$<TARGET_FILE:${_target_NAME}>
            -D_gnupghome="${GNUPGHOME}"
            -P ${CMAKE_SOURCE_DIR}/cmake/modules/kde4_crypto_exec_via_sh.cmake
            )
      else ()
         add_custom_command(TARGET ${_target_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            -D_filename=${_executable}.shell -D_library_path_variable=${_library_path_variable}
            -D_ld_library_path="${_ld_library_path}" -D_executable=${_executable}
            -D_gnupghome="${GNUPGHOME}"
            -P ${CMAKE_SOURCE_DIR}/cmake/modules/kde4_crypto_exec_via_sh.cmake
            )
      endif ()

      # REACTIVATE IT macro_additional_clean_files(${_executable}.shell)

      # under UNIX, set the property WRAPPER_SCRIPT to the name of the generated shell script
      # so it can be queried and used later on easily
      set_target_properties(${_target_NAME} PROPERTIES WRAPPER_SCRIPT ${_executable}.shell)
   else (UNIX)
      # under windows, set the property WRAPPER_SCRIPT just to the name of the executable
      # maybe later this will change to a generated batch file (for setting the PATH so that the Qt libs are found)
      get_target_property(_executable ${_target_NAME} LOCATION )
      set_target_properties(${_target_NAME} PROPERTIES WRAPPER_SCRIPT ${_executable})

      set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}\;${LIB_INSTALL_DIR}\;${KDE4_LIB_DIR}\;${QT_LIBRARY_DIR}")
      get_target_property(_executable ${_target_NAME} LOCATION )

      # use add_custom_target() to have the batch-file-wrapper generated during build time instead of cmake time
      add_custom_command(TARGET ${_target_NAME} POST_BUILD
         COMMAND ${CMAKE_COMMAND}
         -D_filename="${_executable}.bat"
         -D_ld_library_path="${_ld_library_path}" -D_executable="${_executable}"
         -D_gnupghome="${GNUPGHOME}"
         -P ${CMAKE_SOURCE_DIR}/cmake/modules/kde4_crypto_exec_via_sh.cmake
         )

   endif (UNIX)
endmacro (KDE4_HANDLE_CRYPTO_RPATH_FOR_EXECUTABLE)

