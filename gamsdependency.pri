win32 {
    # Switch off warings caused by GAMS headers
    DEFINES += _CRT_SECURE_NO_WARNINGS
}

include (version)
# GAMS_CORE_PATH is Jenkins build switch
GAMS_CORE_TMP = $$(GAMS_CORE_PATH)
!exists($$PWD/gamsinclude.pri) {
    equals(GAMS_CORE_TMP, "") {
        macx {
            GAMSINC = GAMS_DISTRIB=/Library/Frameworks/GAMS.framework/Versions/'$$GAMS_DISTRIB_MAJOR'/Resources \
                      GAMS_DISTRIB_C_API=\$$GAMS_DISTRIB/apifiles/C/api   \
                      GAMS_DISTRIB_CPP_API=\$$GAMS_DISTRIB/apifiles/C++/api
        }
        unix:!macx {
            GAMSINC = GAMS_DISTRIB=$$(HOME)/gams/'$$GAMS_DISTRIB_MAJOR'.'$$GAMS_DISTRIB_MINOR' \
                      GAMS_DISTRIB_C_API=\$$GAMS_DISTRIB/apifiles/C/api   \
                      GAMS_DISTRIB_CPP_API=\$$GAMS_DISTRIB/apifiles/C++/api
        }
        win32 {
            GAMSINC = GAMS_DISTRIB=C:/GAMS/win64/'$$GAMS_DISTRIB_MAJOR'.'$$GAMS_DISTRIB_MINOR' \
                      GAMS_DISTRIB_C_API=\$$GAMS_DISTRIB/apifiles/C/api   \
                      GAMS_DISTRIB_CPP_API=\$$GAMS_DISTRIB/apifiles/C++/api
        }
        write_file($$PWD/gamsinclude.pri,GAMSINC)
    } else {
        GAMSINC = GAMS_DISTRIB=$$(GAMS_CORE_PATH)   \
                  GAMS_DISTRIB_C_API=\$$GAMS_DISTRIB/apifiles/C/api       \
                  GAMS_DISTRIB_CPP_API=\$$GAMS_DISTRIB/apifiles/C++/api
        write_file($$PWD/gamsinclude.pri,GAMSINC)
    }
}
exists($$PWD/gamsinclude.pri) {
    include($$PWD/gamsinclude.pri)
    macx {
        DEFINES += 'GAMS_DISTRIB_PATH=\\"$$GAMS_DISTRIB\\"'
    }
    else {
        DEFINES += 'GAMS_DISTRIB_PATH=\\"\\"'
    }
}

INCLUDEPATH += $$GAMS_DISTRIB_C_API     \
               $$GAMS_DISTRIB_CPP_API

SOURCES += \
    $$GAMS_DISTRIB_C_API/gevmcc.c   \
    $$GAMS_DISTRIB_C_API/gmomcc.c   \
    $$GAMS_DISTRIB_C_API/dctmcc.c   \
    $$GAMS_DISTRIB_C_API/palmcc.c   \
    $$GAMS_DISTRIB_C_API/gdxcc.c    \
    $$GAMS_DISTRIB_C_API/optcc.c    \
    $$GAMS_DISTRIB_C_API/cfgmcc.c   \
    $$GAMS_DISTRIB_C_API/guccc.c    \
    $$GAMS_DISTRIB_C_API/gucapi.c
