
add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set(kgroupwarebase_LIB_SRCS 
   folderlister.cpp 
   folderconfig.cpp 
   folderlistview.cpp 
   folderselectdialog.cpp 
   groupwaredataadaptor.cpp 
   groupwaredownloadjob.cpp 
   calendaradaptor.cpp 
   addressbookadaptor.cpp 
   groupwareresourcejob.cpp 
   groupwareuploadjob.cpp 
   kcal_resourcegroupwarebase.cpp 
   kabc_resourcegroupwarebase.cpp 
   kcal_resourcegroupwarebaseconfig.cpp 
   kabc_resourcegroupwarebaseconfig.cpp 
   kcal_cachesettingsdlg.cpp )


kde4_add_kcfg_files(kgroupwarebase_LIB_SRCS kresources_groupwareprefs.kcfgc )

kde4_add_library(kgroupwarebase SHARED ${kgroupwarebase_LIB_SRCS})

target_link_libraries(kgroupwarebase  ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KCAL_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${QT_QT3SUPPORT_LIBRARY} kdepim )

set_target_properties(kgroupwarebase PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kgroupwarebase ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kgroupwaredav_LIB_SRCS 
   davcalendaradaptor.cpp 
   davaddressbookadaptor.cpp 
   webdavhandler.cpp 
   davgroupwareglobals.cpp )


kde4_add_library(kgroupwaredav SHARED ${kgroupwaredav_LIB_SRCS})

target_link_libraries(kgroupwaredav  ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KCAL_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kdepim kgroupwarebase )
target_link_libraries(kgroupwaredav  LINK_INTERFACE_LIBRARIES  kgroupwarebase )

set_target_properties(kgroupwaredav
   PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS kgroupwaredav ${INSTALL_TARGETS_DEFAULT_ARGS})

