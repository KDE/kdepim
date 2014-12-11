
set( mailfilteragent_pop3resourceattributetest_source pop3resourceattributetest.cpp ../pop3resourceattribute.cpp)
kde4_add_unit_test( pop3resourceattributetest ${mailfilteragent_pop3resourceattributetest_source})
target_link_libraries( pop3resourceattributetest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS})

