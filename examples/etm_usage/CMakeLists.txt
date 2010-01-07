project(etm_usage)

set(example_mail_SRCS
    main.cpp
    mainwindow.cpp
)

kde4_add_executable(etm_usage ${etm_usage_SRCS} )

target_link_libraries(etm_usage
                      ${KDEPIMLIBS_AKONADI_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${KDEPIMLIBS_KABC_LIBS}
                      ${KDEPIMLIBS_KMIME_LIBS}
)

install(TARGETS etm_usage
        DESTINATION ${BIN_INSTALL_DIR})

