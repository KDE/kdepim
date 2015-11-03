set(testspellchecklineedit_SRCS testspellchecklineedit.cpp)

add_executable(testspellchecklineedit ${testspellchecklineedit_SRCS})
ecm_mark_as_test(testspellchecklineedit)

target_link_libraries(testspellchecklineedit KF5::PimCommon KF5::PimTextEdit)

set(customtoolswidgetng_gui_SRCS customtoolswidgetng_gui.cpp)
add_executable(customtoolswidgetng_gui ${customtoolswidgetng_gui_SRCS})
target_link_libraries(customtoolswidgetng_gui
  KF5::PimCommon
  KF5::XmlGui
)

