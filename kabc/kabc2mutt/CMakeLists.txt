
set(kabc2mutt_SRCS kabc2mutt.cpp main.cpp)

kde4_automoc(${kabc2mutt_SRCS})

kde4_add_executable(kabc2mutt ${kabc2mutt_SRCS})

# kdeui for KApplication
target_link_libraries(kabc2mutt  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS kabc2mutt  DESTINATION ${BIN_INSTALL_DIR})
