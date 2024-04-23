/**
 * GAMS Model Instance Inspector (MII)
 *
 * Copyright (c) 2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2023 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include <QtTest>

#include "datahandler.h"
#include "abstractmodelinstance.h"

using namespace gams::studio::mii;

class TestDataHandler : public QObject
{
    Q_OBJECT

private slots:
    void test_DataHandler_empty();
};

void TestDataHandler::test_DataHandler_empty()
{
    EmptyModelInstance modelInstance;
    DataHandler dataHandler(modelInstance);
    QCOMPARE(dataHandler.data(-4, -2, -1), QVariant());
    QCOMPARE(dataHandler.data(4, 2, -1), QVariant());
    QCOMPARE(dataHandler.data(4, 2, 0), QVariant());
    QCOMPARE(dataHandler.data(4, 2, 84), QVariant());
    QCOMPARE(dataHandler.headerData(-1, Qt::Horizontal, -1), -1);
    QCOMPARE(dataHandler.headerData(-1, Qt::Vertical, -1), -1);
    QCOMPARE(dataHandler.headerData(1, Qt::Horizontal, 1), -1);
    QCOMPARE(dataHandler.headerData(1, Qt::Vertical, 1), -1);
    QCOMPARE(dataHandler.rowCount(-1), 0);
    QCOMPARE(dataHandler.rowCount(0), 0);
    QCOMPARE(dataHandler.rowEntryCount(-1, -1), 0);
    QCOMPARE(dataHandler.rowEntryCount(0, 0), 0);
    QCOMPARE(dataHandler.columnCount(-1), 0);
    QCOMPARE(dataHandler.columnCount(0), 0);
    QCOMPARE(dataHandler.columnEntryCount(-1, -1), 0);
    QCOMPARE(dataHandler.columnEntryCount(0, 0), 0);
    QCOMPARE(dataHandler.maxSymbolDimension(0, Qt::Horizontal), 0);
    QCOMPARE(dataHandler.maxSymbolDimension(0, Qt::Vertical), 0);
    dataHandler.loadJacobian();
}

QTEST_APPLESS_MAIN(TestDataHandler)

#include "tst_testdatahandler.moc"
