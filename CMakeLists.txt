project(kdepim)

# where to look first for cmake modules. This line must be the first one or cmake will use the system's FindFoo.cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")


############### search packages used by KDE ###############

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(CheckIncludeFiles)
include(MacroBoolTo01)
find_package(KdepimLibs 4.3.85 REQUIRED)


############### Load the CTest options ###############
# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake)


############### search Boost ###############

find_package(Boost 1.34.0)
macro_log_feature(Boost_FOUND "Boost" "Boost C++ Libraries" "http://www.boost.org" TRUE "1.34.0" "Required by several critical KDEPIM apps.")



############### Windows specific ###############

if(WIN32)
  # detect oxygen icon dir at configure time based on KDEDIRS - there may be different package installation locations
  execute_process(COMMAND "${KDE4_KDECONFIG_EXECUTABLE}" --path icon OUTPUT_VARIABLE _dir ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
  file(TO_CMAKE_PATH "${_dir}" __dir)
  find_path(KDE4_ICON_DIR oxygen PATHS
    ${__dir}
  )
  message(STATUS "using oxygen application icons from ${KDE4_ICON_DIR}")
else(WIN32)
    set (KDE4_ICON_DIR  ${CMAKE_INSTALL_PREFIX}/share/icons)
endif(WIN32)


############### Find the stuff we need ###############

  find_package(Akonadi QUIET NO_MODULE)
  macro_log_feature(Akonadi_FOUND "Akonadi" "Akonadi server libraries" "http://pim.kde.org/akonadi" TRUE "" "Akonadi is required to build KDEPIM")
  find_package(ZLIB)
  macro_log_feature(ZLIB_FOUND "ZLib" "The Zlib compression library" "http://www.zlib.net" TRUE "" "")

  find_package(Strigi)
  macro_log_feature(STRIGI_FOUND "Strigi" "Index metadata of files" "http://strigi.sourceforge.net" TRUE "" "")

  macro_optional_find_package(Grantlee)
  macro_log_feature(Grantlee_FOUND "Grantlee" "The Grantlee Template System" "http://www.grantlee.org" FALSE "" "Templating and theming for KJots." )

  set(SDO_MIN_VERSION 0.2)
  set(SOPRANO_MIN_VERSION 2.3.70)

  find_package(SharedDesktopOntologies ${SDO_MIN_VERSION})
  macro_log_feature(SHAREDDESKTOPONTOLOGIES_FOUND "Shared desktop ontologies" "Desktop ontologies" "http://oscaf.sourceforge.net" TRUE "${SDO_MIN_VERSION}" "Ontologies necessary for the Nepomuk semantic desktop.")

  find_package(Soprano)
  macro_log_feature(Soprano_FOUND "Soprano" "Semantic Desktop Storing" "http://soprano.sourceforge.net" TRUE "${SOPRANO_MIN_VERSION}" "Soprano is needed for Nepomuk")
  if(Soprano_FOUND)
    include(SopranoAddOntology)
  endif(Soprano_FOUND)

  find_package(Nepomuk)
  macro_log_feature(Nepomuk_FOUND "Nepomuk" "The Nepomuk libraries" "http://www.kde.org" TRUE "" "Nepomuk extends the search and tagging functionalities in KMail and Akonadi")

  find_program(XSLTPROC_EXECUTABLE xsltproc)
  macro_log_feature(XSLTPROC_EXECUTABLE "xsltproc" "The command line XSLT processor from libxslt" "http://xmlsoft.org/XSLT/" TRUE "" "Required to generate the D-Bus interfaces.")

############### Needed commands before building anything ###############

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${KDEPIM_DEFINITIONS})

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDEPIMLIBS_INCLUDE_DIRS} ${KDE4_INCLUDES} ${QT_QTDBUS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libkdepim)



add_subdirectory( ontologies ) # must be the first one.
add_subdirectory(libkdepim)
#  add_subdirectory(libkdepimdbusinterfaces)
add_subdirectory(krss)
add_subdirectory(akregator)



