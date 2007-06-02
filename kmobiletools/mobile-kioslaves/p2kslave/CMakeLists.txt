
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kio_p2k_PART_SRCS kio_p2kprotocol.cpp p2kimpl.cpp p2kwrapper.cpp )

kde4_automoc(${kio_p2k_PART_SRCS})

#kde4_add_dcop_stubs(kio_p2k_PART_SRCS MESSAGE(STATUS "DCOP stuff was remove into kde4, convert it to dbus please. kde4_add_dcop_stubs doen't exist. We keep it to remember to port to dbus")
deviceIFace.h )

kde4_add_plugin(kio_p2k ${kio_p2k_PART_SRCS})

target_link_libraries(kio_p2k  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} p2kmoto kmobiletools )

install(TARGETS kio_p2k DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  p2k.protocol DESTINATION ${SERVICES_INSTALL_DIR} )



#original Makefile.am contents follow:

#KDE_OPTIONS = nofinal
#
#INCLUDES = -I$(srcdir)/../../kmobiletools/libkmobiletools \
#	-I../../kmobiletools/libkmobiletools \
#	$(all_includes)
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = kio_p2k.la
#
#kio_p2k_la_SOURCES = kio_p2kprotocol.cpp p2kimpl.cpp p2kwrapper.cpp deviceIFace.stub
#kio_p2k_la_LDFLAGS = -module -no-undefined $(all_libraries) $(KDE_PLUGIN)
#kio_p2k_la_LIBADD = -lkio -lp2kmoto ../../kmobiletools/libkmobiletools/libkmobiletools.la
#
#protocol_DATA = p2k.protocol
#protocoldir = $(kde_servicesdir)
#
#deviceIFace_DIR=$(srcdir)/../../kmobiletools/libkmobiletools
#
#
#
#noinst_HEADERS = kio_p2kprotocol.h p2kimpl.h p2kwrapper.h
#
#KDE_CXXFLAGS = -UQT_NO_ASCII_CAST
#
