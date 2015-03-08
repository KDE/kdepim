
set( mailcommon_newmailnotifierattribute_source newmailnotifierattributetest.cpp ../attributes/newmailnotifierattribute.cpp)
kde4_add_unit_test( newmailnotifierattributetest ${mailcommon_newmailnotifierattribute_source})
target_link_libraries( newmailnotifierattributetest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS})


set( mailcommon_collectionannotationsattributetest_source collectionannotationsattributetest.cpp ../attributes/collectionannotationsattribute.cpp)
kde4_add_unit_test( collectionannotationsattributetest ${mailcommon_collectionannotationsattributetest_source})
target_link_libraries( collectionannotationsattributetest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS})


set( mailcommon_expirecollectionattributetest_source expirecollectionattributetest.cpp ../attributes/expirecollectionattribute.cpp )
kde4_add_unit_test( expirecollectionattributetest ${mailcommon_expirecollectionattributetest_source})
target_link_libraries( expirecollectionattributetest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS})

set( mailcommon_incidencesforwidgettest_source incidencesforwidgettest.cpp )
kde4_add_unit_test( incidencesforwidgettest ${mailcommon_incidencesforwidgettest_source})
target_link_libraries( incidencesforwidgettest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} mailcommon)

set( mailcommon_contenttypewidgettest_source contenttypewidgettest.cpp )
kde4_add_unit_test( contenttypewidgettest ${mailcommon_contenttypewidgettest_source})
target_link_libraries( contenttypewidgettest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} mailcommon)


