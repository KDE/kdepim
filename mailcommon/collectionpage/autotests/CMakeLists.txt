
set( mailcommon_newmailnotifierattribute_source newmailnotifierattributetest.cpp ../newmailnotifierattribute.cpp)
kde4_add_unit_test( newmailnotifierattributetest ${mailcommon_newmailnotifierattribute_source})
target_link_libraries( newmailnotifierattributetest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS})


set( mailcommon_collectionannotationsattributetest_source collectionannotationsattributetest.cpp ../collectionannotationsattribute.cpp)
kde4_add_unit_test( collectionannotationsattributetest ${mailcommon_collectionannotationsattributetest_source})
target_link_libraries( collectionannotationsattributetest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS})


set( mailcommon_expirecollectionattributetest_source expirecollectionattributetest.cpp ../expirecollectionattribute.cpp )
kde4_add_unit_test( expirecollectionattributetest ${mailcommon_expirecollectionattributetest_source})
target_link_libraries( expirecollectionattributetest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS})

set( mailcommon_contentstypecomboboxtest_source contentstypecomboboxtest.cpp )
kde4_add_unit_test( contentstypecomboboxtest ${mailcommon_contentstypecomboboxtest_source})
target_link_libraries( contentstypecomboboxtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} mailcommon)

