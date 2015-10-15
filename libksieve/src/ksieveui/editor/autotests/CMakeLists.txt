set( libksieveui_sieveeditormenubartest_source sieveeditormenubartest.cpp ../sieveeditormenubar.cpp )
add_executable( sieveeditormenubartest ${libksieveui_sieveeditormenubartest_source})
add_test(sieveeditormenubartest sieveeditormenubartest)
ecm_mark_as_test(sieveeditormenubartest)
target_link_libraries( sieveeditormenubartest Qt5::Test Qt5::Gui Qt5::Widgets KF5::ConfigWidgets KF5::I18n KF5::PimTextEdit)



set( libksieveui_sievetextedittest_source sievetextedittest.cpp )
add_executable( sievetextedittest ${libksieveui_sievetextedittest_source})
add_test(sievetextedittest sievetextedittest)
ecm_mark_as_test(sievetextedittest)
target_link_libraries( sievetextedittest Qt5::Test  Qt5::Gui KF5::KSieveUi KF5::PimCommon KF5::PimTextEdit)


set( libksieveui_sievetexteditwidgettest_source sievetexteditwidgettest.cpp )
add_executable( sievetexteditwidgettest ${libksieveui_sievetexteditwidgettest_source})
add_test(sievetexteditwidgettest sievetexteditwidgettest)
ecm_mark_as_test(sievetexteditwidgettest)
target_link_libraries( sievetexteditwidgettest Qt5::Test  Qt5::Gui KF5::KSieveUi KF5::PimCommon KF5::PimTextEdit)
