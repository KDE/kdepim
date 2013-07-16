project(xmlsieveparsing)

set(xmlsieveparsing_SRCS
  main.cpp
  xmlprintingscriptbuilder.cpp
)

KDE4_ADD_EXECUTABLE(xmlsieveparsing ${xmlsieveparsing_SRCS} )
TARGET_LINK_LIBRARIES(xmlsieveparsing ${KDE4_KDEUI_LIBS}  ksieve)

