include(../tests.pri)

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$SRCPATH/modelinspector

SOURCES +=  tst_testfiltertreeitem.cpp                  \
            $$SRCPATH/modelinspector/filtertreeitem.cpp
