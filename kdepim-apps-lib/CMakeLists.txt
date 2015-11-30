project(kdepim-apps-lib)
cmake_minimum_required(VERSION 2.8.12)



add_subdirectory(composereditor-ng)
add_subdirectory(libfollowupreminder)
add_subdirectory(libkdepimdbusinterfaces)
add_subdirectory(libsendlater)
add_subdirectory(kaddressbookgrantlee)

install( FILES kdepim-apps-lib.categories DESTINATION ${KDE_INSTALL_CONFDIR} )
