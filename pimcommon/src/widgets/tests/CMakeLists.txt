set(customtextedit_gui_SRCS customtextedit_gui.cpp)
add_executable(customtextedit_gui ${customtextedit_gui_SRCS})
target_link_libraries(customtextedit_gui
  KF5::PimCommon
)

set(slidecontainer_gui_SRCS slidecontainer_gui.cpp)
add_executable(slidecontainer_gui ${slidecontainer_gui_SRCS})
target_link_libraries(slidecontainer_gui
  Qt5::Core
  Qt5::Gui
  KF5::PimCommon
)

set(testspellchecklineedit_SRCS testspellchecklineedit.cpp)

add_executable(testspellchecklineedit ${testspellchecklineedit_SRCS})
ecm_mark_as_test(testspellchecklineedit)

target_link_libraries(testspellchecklineedit KF5::PimCommon )

set(customtoolswidgetng_gui_SRCS customtoolswidgetng_gui.cpp)
add_executable(customtoolswidgetng_gui ${customtoolswidgetng_gui_SRCS})
target_link_libraries(customtoolswidgetng_gui
  KF5::PimCommon
  KF5::XmlGui
)

