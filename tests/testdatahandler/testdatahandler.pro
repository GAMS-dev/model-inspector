include(../tests.pri)

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$SRCPATH/modelinspector

SOURCES +=  tst_testdatahandler.cpp                                 \
            $$SRCPATH/modelinspector/datahandler.cpp                \
            $$SRCPATH/modelinspector/modelinstance.cpp              \
            $$SRCPATH/modelinspector/abstractmodelinstance.cpp      \
            $$SRCPATH/modelinspector/aggregation.cpp                \
            $$SRCPATH/modelinspector/symbol.cpp                     \
            $$SRCPATH/modelinspector/labeltreeitem.cpp              \
            $$SRCPATH/modelinspector/viewconfigurationprovider.cpp  \
            $$SRCPATH/modelinspector/common.cpp                     \
            $$SRCPATH/modelinspector/postopttreeitem.cpp
