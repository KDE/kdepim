add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories(${CMAKE_SOURCE_DIR}/libksieve
	${CMAKE_SOURCE_DIR}/pimcommon
	${CMAKE_BINARY_DIR}/pimcommon
        ${CMAKE_SOURCE_DIR}/libksieve/ksieveui
	)

add_subdirectory(tests)

set(ksieveui_LIB_SRCS
  managesievescriptsdialog.cpp
  debug/sievedebugdialog.cpp
  util.cpp
  vacation/vacation.cpp
  vacation/vacationdialog.cpp
  editor/sievefindbar.cpp
  editor/sievesyntaxhighlighter.cpp
  editor/sievetextedit.cpp
  editor/sieveeditor.cpp
  editor/sievelinenumberarea.cpp
  templates/sievetemplatewidget.cpp
  templates/sievedefaulttemplate.cpp 
  templates/sievetemplateeditdialog.cpp 
  autocreatescripts/autocreatescriptdialog.cpp
  autocreatescripts/sieveconditionwidgetlister.cpp
  autocreatescripts/sievescriptlistbox.cpp
  autocreatescripts/sievescriptdescriptiondialog.cpp
  autocreatescripts/sieveactionwidgetlister.cpp
  autocreatescripts/sievescriptpage.cpp
  autocreatescripts/autocreatescriptutil.cpp
  autocreatescripts/sieveactions/sieveactionlist.cpp
  autocreatescripts/sieveactions/sieveaction.cpp
  autocreatescripts/sieveactions/sieveactiondiscard.cpp
  autocreatescripts/sieveactions/sieveactionstop.cpp
  autocreatescripts/sieveactions/sieveactionsetflags.cpp
  autocreatescripts/sieveactions/sieveactionaddflags.cpp
  autocreatescripts/sieveactions/sieveactionfileinto.cpp
  autocreatescripts/sieveactions/sieveactionreject.cpp
  autocreatescripts/sieveactions/sieveactionkeep.cpp
  autocreatescripts/sieveactions/sieveactionredirect.cpp
  autocreatescripts/sieveactions/sieveactionabstractflags.cpp
  autocreatescripts/sieveactions/sieveactionremoveflags.cpp
  autocreatescripts/sieveactions/sieveactionnotify.cpp
  autocreatescripts/sieveactions/widgets/selectflagswidget.cpp
  autocreatescripts/sieveactions/widgets/selectfileintowidget.cpp
  autocreatescripts/sieveactions/widgets/addresslineedit.cpp
  autocreatescripts/sieveconditions/sievecondition.cpp
  autocreatescripts/sieveconditions/sieveconditionheader.cpp
  autocreatescripts/sieveconditions/sieveconditionlist.cpp
  autocreatescripts/sieveconditions/sieveconditionaddress.cpp
  autocreatescripts/sieveconditions/sieveconditionsize.cpp
  autocreatescripts/sieveconditions/sieveconditionenvelope.cpp
  autocreatescripts/sieveconditions/sieveconditionexists.cpp
  autocreatescripts/sieveconditions/sieveconditiontrue.cpp
  autocreatescripts/sieveconditions/sieveconditionbody.cpp
  autocreatescripts/sieveconditions/widgets/selectmatchtypecombobox.cpp
  autocreatescripts/sieveconditions/widgets/selectaddresspartcombobox.cpp
  autocreatescripts/sieveconditions/widgets/selectheadertypecombobox.cpp
  autocreatescripts/sieveconditions/widgets/selectbodytypewidget.cpp
)

kde4_add_kcfg_files(ksieveui_LIB_SRCS settings.kcfgc)

kde4_add_library(ksieveui ${LIBRARY_TYPE} ${ksieveui_LIB_SRCS})

target_link_libraries(ksieveui
  kmanagesieve
  ksieve
  pimcommon
  kdepim
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDE4_KDECORE_LIBS}
)

set_target_properties(
  ksieveui PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS ksieveui ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(autocreatescripts/tests)
