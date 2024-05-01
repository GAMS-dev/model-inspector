include(../tests.pri)

QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$SRCPATH/mii

SOURCES +=  tst_testviewconfigurationprovider.cpp        \
            $$SRCPATH/mii/abstractmodelinstance.cpp      \
            $$SRCPATH/mii/modelinstance.cpp              \
            $$SRCPATH/mii/datahandler.cpp                \
            $$SRCPATH/mii/datamatrix.cpp                 \
            $$SRCPATH/mii/labeltreeitem.cpp              \
            $$SRCPATH/mii/symbol.cpp                     \
            $$SRCPATH/mii/viewconfigurationprovider.cpp  \
            $$SRCPATH/mii/common.cpp                     \
            $$SRCPATH/mii/postopttreeitem.cpp            \
            $$SRCPATH/mii/numerics.cpp
