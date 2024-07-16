#
# This file is part of the GAMS Studio project.
#
# Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
# Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

QT       += core gui widgets svg concurrent

TARGET = mii
TEMPLATE = app
DESTDIR = bin
CONFIG += c++17

# Setup and include the GAMS distribution
include(../gamsdependency.pri)

INCLUDEPATH += $$PWD/../extern
include(../extern/dtoaloc/dtoaloc.pri)

macx {
# ! The icns-file is created from a folder named gams.iconset containing images in multiple sizes.
# ! On mac osX type the command: iconutil -c icns [base-folder]/gams.iconset to create gams.icns
    ICON = ../icons/miilogo.icns

    HEADERS += macoscocoabridge.h \
               macospathfinder.h

    SOURCES += macospathfinder.cpp

    OBJECTIVE_SOURCES += macoscocoabridge.mm

    LIBS += -framework AppKit

    QMAKE_INFO_PLIST = ../platform/macos/info.plist
}
unix {
    LIBS += -ldl
}
win32 {
    RC_ICONS = ../icons/miilogo.ico
    LIBS += -luser32
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    exception.cpp \
    logview.cpp \
    main.cpp \
    mainwindow.cpp \
    gamsprocess.cpp \
    commonpaths.cpp \
    gamslibprocess.cpp \
    mii/abstractmodelinstance.cpp \
    mii/abstracttableview.cpp \
    mii/abstracttableviewframe.cpp \
    mii/abstractviewframe.cpp \
    mii/bpidentifierfiltermodel.cpp \
    mii/bpviewframe.cpp \
    mii/common.cpp \
    mii/comprehensivetablemodel.cpp \
    mii/datahandler.cpp \
    mii/datamatrix.cpp \
    mii/dtoaformatproxymodel.cpp \
    mii/filterdialog.cpp \
    mii/filtertreeitem.cpp \
    mii/filtertreemodel.cpp \
    mii/hierarchicalheaderview.cpp \
    mii/labeltreeitem.cpp \
    mii/modelinstance.cpp    \
    mii/modelinspector.cpp \
    mii/modelinstancetableview.cpp \
    mii/numerics.cpp \
    mii/postopttreeitem.cpp \
    mii/postopttreemodel.cpp \
    mii/postopttreeview.cpp \
    mii/postopttreeviewframe.cpp \
    mii/search.cpp \
    mii/searchresultmodel.cpp \
    mii/sectiontreeitem.cpp \
    mii/sectiontreemodel.cpp \
    mii/sectiontreeview.cpp \
    mii/symbol.cpp \
    mii/symbolfiltermodel.cpp \
    mii/symbolhierarchicalheaderview.cpp \
    mii/symbolmodelinstancetablemodel.cpp \
    mii/symbolviewframe.cpp \
    mii/valueformatproxymodel.cpp \
    mii/searchresultview.cpp \
    mii/viewconfigurationprovider.cpp

HEADERS += \
    exception.h \
    logview.h \
    mainwindow.h \
    gamsprocess.h \
    commonpaths.h \
    gamslibprocess.h    \
    mii/abstractmodelinstance.h \
    mii/abstracttableview.h \
    mii/abstracttableviewframe.h \
    mii/abstractviewframe.h \
    mii/bpidentifierfiltermodel.h \
    mii/bpviewframe.h \
    mii/common.h \
    mii/comprehensivetablemodel.h \
    mii/datahandler.h \
    mii/datamatrix.h \
    mii/dtoaformatproxymodel.h \
    mii/filterdialog.h \
    mii/filtertreeitem.h \
    mii/filtertreemodel.h \
    mii/hierarchicalheaderview.h \
    mii/labeltreeitem.h \
    mii/modelinstance.h  \
    mii/modelinspector.h \
    mii/modelinstancetableview.h \
    mii/numerics.h \
    mii/postopttreeitem.h \
    mii/postopttreemodel.h \
    mii/postopttreeview.h \
    mii/postopttreeviewframe.h \
    mii/search.h \
    mii/searchresultmodel.h \
    mii/sectiontreeitem.h \
    mii/sectiontreemodel.h \
    mii/sectiontreeview.h \
    mii/symbol.h \
    mii/symbolfiltermodel.h \
    mii/symbolhierarchicalheaderview.h \
    mii/symbolmodelinstancetablemodel.h \
    mii/symbolviewframe.h \
    mii/valueformatproxymodel.h \
    mii/searchresultview.h \
    mii/viewconfigurationprovider.h

FORMS += \
    mainwindow.ui \
    mii/filterdialog.ui \
    mii/modelinspector.ui \
    mii/postopttreeviewframe.ui \
    mii/standardtableviewframe.ui

RESOURCES += \
    ../icons/icons.qrc

OTHER_FILES +=                                      \
    ../platform/linux/mii.desktop                   \
    ../platform/macos/gams-mii.entitlements.plist   \
    ../ci/cloudfront-deploy.sh                      \
    ../ci/github-deploy.sh                          \
    ../ci/codechecker.sh                            \
    ../ci/skipfile.txt                              \
    ../ci/.gitlab-ci-06-gams.yml                    \
    ../ci/.gitlab-ci-09-build.yml                   \
    ../ci/.gitlab-ci-12-test.yml                    \
    ../ci/.gitlab-ci-15-pack.yml                    \
    ../ci/.gitlab-ci-20-analyze.yml                 \
    ../ci/.gitlab-ci-24-deploy.yml                  \
    ../.gitlab-ci.yml                               \
    ../version
