kde4_add_unit_test(contactfieldstest NOGUI contactfieldstest.cpp ../contactfields.cpp)
target_link_libraries(contactfieldstest ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${QT_QTTEST_LIBRARY})
