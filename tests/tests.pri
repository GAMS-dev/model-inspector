QT += core gui widgets testlib

CONFIG += c++17
CONFIG -= app_bundle

DESTDIR = ../bin

# Setup and include the GAMS distribution
include(../gamsdependency.pri)

INCLUDEPATH += $$PWD/../extern
include(../extern/dtoaloc/dtoaloc.pri)

macx {
    HEADERS += ../../src/macospathfinder.h
               ../../src/macoscocoabridge.h

    SOURCES += ../../src/macospathfinder.cpp

    OBJECTIVE_SOURCES += ../../src/macoscocoabridge.mm

    LIBS += -framework AppKit
}
unix {
    LIBS += -ldl
}
win32 {
    LIBS += -luser32
}

TESTSROOT = $$_PRO_FILE_PWD_/..
SRCPATH = $$TESTSROOT/../src
