add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
if(KDEPIM_ENTERPRISE_BUILD)
  set(USE_GRAPHICAL_SIEVE_EDITOR true)
else()
  set(USE_GRAPHICAL_SIEVE_EDITOR false)
endif()


configure_file(settings/sieve-editor.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/sieve-editor.kcfg)

include_directories(${CMAKE_SOURCE_DIR}/libksieve
	${CMAKE_SOURCE_DIR}/pimcommon
        ${CMAKE_SOURCE_DIR}/pimcommon/sievehighlighter/
	${CMAKE_BINARY_DIR}/pimcommon
        ${CMAKE_SOURCE_DIR}/pimcommon/util/
        ${CMAKE_SOURCE_DIR}/libksieve/ksieveui
	${CMAKE_SOURCE_DIR}/libkdepim/
	)

add_subdirectory(tests)
add_subdirectory(editor/tests)
add_subdirectory(managescriptsjob/tests)
set(ksieveui_LIB_SRCS
  scriptsparsing/xmlprintingscriptbuilder.cpp
  scriptsparsing/parsingresultdialog.cpp
  scriptsparsing/xmlprintingsyntaxhighlighter.cpp
  scriptsparsing/parsingutil.cpp
  managesievescriptsdialog.cpp
  widgets/sievetreewidgetitem.cpp
  widgets/managesievetreeview.cpp
  debug/sievedebugdialog.cpp
  util/util.cpp
  vacation/vacation.cpp
  vacation/vacationdialog.cpp
  vacation/vacationhelperjob.cpp
  vacation/vacationmanager.cpp
  managescriptsjob/generateglobalscriptjob.cpp
  managescriptsjob/parseuserscriptjob.cpp
  editor/sievetextedit.cpp
  editor/sieveeditor.cpp
  editor/sievelinenumberarea.cpp
  editor/sieveinfowidget.cpp
  editor/sieveeditortextmodewidget.cpp
  editor/sieveeditorabstractwidget.cpp
  editor/sieveeditorwarning.cpp
  editor/sieveeditorparsingmissingfeaturewarning.cpp
  templates/sievetemplatewidget.cpp
  templates/sievedefaulttemplate.cpp 
  templates/sievetemplateeditdialog.cpp
  autocreatescripts/sievescriptparsingerrordialog.cpp
  autocreatescripts/sieveeditorgraphicalmodewidget.cpp 
  autocreatescripts/autocreatescriptdialog.cpp
  autocreatescripts/sieveconditionwidgetlister.cpp
  autocreatescripts/sievescriptlistbox.cpp
  autocreatescripts/sievescriptdescriptiondialog.cpp
  autocreatescripts/sieveactionwidgetlister.cpp
  autocreatescripts/sievescriptpage.cpp
  autocreatescripts/sievescriptblockwidget.cpp
  autocreatescripts/sievescripttabwidget.cpp
  autocreatescripts/autocreatescriptutil.cpp
  autocreatescripts/sieveincludewidget.cpp
  autocreatescripts/sieveforeverypartwidget.cpp
  autocreatescripts/sievewidgetpageabstract.cpp
  autocreatescripts/sieveglobalvariablewidget.cpp
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
  autocreatescripts/sieveactions/sieveactionabstracteditheader.cpp
  autocreatescripts/sieveactions/sieveactiondeleteheader.cpp
  autocreatescripts/sieveactions/sieveactionaddheader.cpp
  autocreatescripts/sieveactions/sieveactionvacation.cpp
  autocreatescripts/sieveactions/sieveactionenclose.cpp
  autocreatescripts/sieveactions/sieveactionreplace.cpp
  autocreatescripts/sieveactions/sieveactionextracttext.cpp
  autocreatescripts/sieveactions/sieveactionbreak.cpp
  autocreatescripts/sieveactions/sieveactionconvert.cpp
  autocreatescripts/sieveactions/sieveactionsetvariable.cpp
  autocreatescripts/sieveactions/sieveactionreturn.cpp
  autocreatescripts/sieveactions/widgets/selectflagswidget.cpp
  autocreatescripts/sieveactions/widgets/selectfileintowidget.cpp
  autocreatescripts/sieveactions/widgets/addresslineedit.cpp
  autocreatescripts/sieveactions/widgets/selectaddheaderpositioncombobox.cpp
  autocreatescripts/sieveactions/widgets/selectimportancecombobox.cpp
  autocreatescripts/sieveactions/widgets/multilineedit.cpp
  autocreatescripts/sieveactions/widgets/selectvacationcombobox.cpp
  autocreatescripts/sieveactions/widgets/selectvariablemodifiercombobox.cpp
  autocreatescripts/commonwidgets/selectconvertparameterwidget.cpp
  autocreatescripts/commonwidgets/selectmatchtypecombobox.cpp
  autocreatescripts/commonwidgets/selectmimetypecombobox.cpp
  autocreatescripts/sieveconditions/sievecondition.cpp
  autocreatescripts/sieveconditions/sieveconditionheader.cpp
  autocreatescripts/sieveconditions/sieveconditionlist.cpp
  autocreatescripts/sieveconditions/sieveconditionaddress.cpp
  autocreatescripts/sieveconditions/sieveconditionsize.cpp
  autocreatescripts/sieveconditions/sieveconditionenvelope.cpp
  autocreatescripts/sieveconditions/sieveconditionexists.cpp
  autocreatescripts/sieveconditions/sieveconditiontrue.cpp
  autocreatescripts/sieveconditions/sieveconditionbody.cpp
  autocreatescripts/sieveconditions/sieveconditiondate.cpp
  autocreatescripts/sieveconditions/sieveconditioncurrentdate.cpp
  autocreatescripts/sieveconditions/sieveconditionmailboxexists.cpp
  autocreatescripts/sieveconditions/sieveconditionspamtest.cpp
  autocreatescripts/sieveconditions/sieveconditionvirustest.cpp
  autocreatescripts/sieveconditions/sieveconditionihave.cpp
  autocreatescripts/sieveconditions/sieveconditionfalse.cpp
  autocreatescripts/sieveconditions/sieveconditionenvironment.cpp
  autocreatescripts/sieveconditions/sieveconditionhasflag.cpp
  autocreatescripts/sieveconditions/sieveconditionmetadata.cpp
  autocreatescripts/sieveconditions/sieveconditionconvert.cpp
  autocreatescripts/sieveconditions/sieveconditionmetadataexists.cpp
  autocreatescripts/sieveconditions/sieveconditionservermetadata.cpp
  autocreatescripts/sieveconditions/sieveconditionservermetadataexists.cpp
  autocreatescripts/sieveconditions/widgets/selectdatewidget.cpp
  autocreatescripts/sieveconditions/widgets/selectaddresspartcombobox.cpp
  autocreatescripts/sieveconditions/widgets/selectheadertypecombobox.cpp
  autocreatescripts/sieveconditions/widgets/selectbodytypewidget.cpp
  autocreatescripts/sieveconditions/widgets/selectrelationalmatchtype.cpp
  autocreatescripts/sieveconditions/widgets/selectcomparatorcombobox.cpp
  autocreatescripts/sieveconditions/widgets/selectmimecombobox.cpp
  autocreatescripts/sieveconditions/widgets/selectsizetypecombobox.cpp
  autocreatescripts/sieveconditions/widgets/selectsizewidget.cpp
)

kde4_add_kcfg_files(ksieveui_LIB_SRCS settings/sieve-vacation.kcfgc settings/sieve-editor.kcfgc)

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
add_subdirectory(scriptsparsing/tests)

